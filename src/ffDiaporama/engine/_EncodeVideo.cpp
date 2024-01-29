/* ======================================================================
    This file is part of ffDiaporama
    ffDiaporama is a tools to make diaporama as video
    Copyright (C) 2011-2014 Dominique Levray <domledom@laposte.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
   ====================================================================== */

#include "_EncodeVideo.h"

#define PIXFMT      AV_PIX_FMT_RGB24
#define QTPIXFMT    QImage::Format_RGB888

//*************************************************************************************************************************************************

int CheckEncoderCapabilities(VFORMAT_ID FormatId,AVCodecID VideoCodec,AVCodecID AudioCodec) {
    if (VideoCodec==AV_CODEC_ID_NONE) return SUPPORTED_COMBINATION;

    int Ret=INVALID_COMBINATION;

    switch (FormatId) {
        case VFORMAT_3GP:
            if (((VideoCodec==AV_CODEC_ID_MPEG4)||(VideoCodec==AV_CODEC_ID_H264))&&((AudioCodec==AV_CODEC_ID_AMR_NB)||(AudioCodec==AV_CODEC_ID_AMR_WB)))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_MJPEG:
            if ((VideoCodec==AV_CODEC_ID_MJPEG)&&(AudioCodec==AV_CODEC_ID_PCM_S16LE))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_OGV:
            if ((VideoCodec==AV_CODEC_ID_THEORA)&&(AudioCodec==AV_CODEC_ID_VORBIS))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_WEBM:
            if ((VideoCodec==AV_CODEC_ID_VP8)&&(AudioCodec==AV_CODEC_ID_VORBIS))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_OLDFLV:
            if ((VideoCodec==AV_CODEC_ID_FLV1)&&(AudioCodec==AV_CODEC_ID_MP3))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_FLV:
            if ((VideoCodec==AV_CODEC_ID_H264)&&(AudioCodec==AV_CODEC_ID_AAC))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_MPEG:
            if ((VideoCodec==AV_CODEC_ID_MPEG2VIDEO)&&((AudioCodec==AV_CODEC_ID_AC3)||(AudioCodec==AV_CODEC_ID_MP2)))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_AVI:
            if (((VideoCodec==AV_CODEC_ID_MPEG4)||(VideoCodec==AV_CODEC_ID_H264))&&
                ((AudioCodec==AV_CODEC_ID_AC3)||(AudioCodec==AV_CODEC_ID_MP3)||(AudioCodec==AV_CODEC_ID_PCM_S16LE)))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_MP4:
            if (((VideoCodec==AV_CODEC_ID_MPEG4)||(VideoCodec==AV_CODEC_ID_H264))&&((AudioCodec==AV_CODEC_ID_MP3)||(AudioCodec==AV_CODEC_ID_AAC)))
                Ret=SUPPORTED_COMBINATION;
            break;
        case VFORMAT_MKV:
            Ret=SUPPORTED_COMBINATION;
            break;
        default: break;
    }
    return Ret;
}

//*************************************************************************************************************************************************

cEncodeVideo::cEncodeVideo() {
    StopProcessWanted       =false;
    Diaporama               =NULL;
    Container               =NULL;
    IsOpen                  =false;

    // Audio
    AudioCodecCtx           =NULL;
    AudioStream             =NULL;
    AudioFrame              =NULL;
    AudioResampler          =NULL;
    AudioResamplerBuffer    =NULL;

    //Video
    VideoCodecCtx           =NULL;
    VideoStream             =NULL;
    VideoEncodeBuffer       =NULL;
    VideoEncodeBufferSize   =40*1024*1024;
    VideoFrameConverter     =NULL;
    VideoFrame              =NULL;
    InternalWidth           =0;
    InternalHeight          =0;
    ExtendV                 =0;
    VideoFrameBufSize       =0;
    VideoFrameBuf           =NULL;
}

//*************************************************************************************************************************************************

cEncodeVideo::~cEncodeVideo() {
    CloseEncoder();
}

//*************************************************************************************************************************************************

void cEncodeVideo::CloseEncoder() {
    if (Container) {
        if (IsOpen) {
            av_write_trailer(Container);
            avio_close(Container->pb);
        }
        avformat_free_context(Container);
        Container=NULL;
    }
    
    if (VideoCodecCtx) avcodec_free_context(&VideoCodecCtx);
    if (AudioCodecCtx) avcodec_free_context(&AudioCodecCtx);
    
    VideoStream=NULL;
    AudioStream=NULL;

    // Audio buffers
    if (AudioFrame) av_freep(&AudioFrame);

    if (AudioResampler) {
        swr_free(&AudioResampler);
        AudioResampler=NULL;
    }
    if (AudioResamplerBuffer) {
        av_free(AudioResamplerBuffer);
        AudioResamplerBuffer=NULL;
    }

    // Video buffers
    if (VideoEncodeBuffer) {
        av_free(VideoEncodeBuffer);
        VideoEncodeBuffer=NULL;
    }
    if (VideoFrameConverter) {
        sws_freeContext(VideoFrameConverter);
        VideoFrameConverter=NULL;
    }
    if (VideoFrame) {
        if ((VideoFrame->extended_data)&&(VideoFrame->extended_data!=VideoFrame->data)) av_freep(&VideoFrame->extended_data);
        if (VideoFrame->data[0]) av_freep(&VideoFrame->data[0]);
        av_freep(&VideoFrame);
    }
}

//*************************************************************************************************************************************************

int cEncodeVideo::getThreadFlags(AVCodecID ID) {
    int Ret=0;
    switch (ID) {
        case AV_CODEC_ID_PRORES:
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_DVVIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:   Ret=FF_THREAD_SLICE;                    break;
        case AV_CODEC_ID_H264 :        Ret=FF_THREAD_FRAME|FF_THREAD_SLICE;    break;
        default:                       Ret=FF_THREAD_FRAME;                    break;
    }
    return Ret;
}

//*************************************************************************************************************************************************

bool cEncodeVideo::OpenEncoder(QWidget *ParentWindow,cDiaporama *Diaporama,QString OutputFileName,int FromSlide,int ToSlide,
                    bool EncodeVideo,int VideoCodecSubId,bool VBR,sIMAGEDEF *ImageDef,int ImageWidth,int ImageHeight,int ExtendV,int InternalWidth,int InternalHeight,AVRational PixelAspectRatio,int VideoBitrate,
                    bool EncodeAudio,int AudioCodecSubId,int AudioChannels,int AudioBitrate,int AudioSampleRate,QString Language) {

    this->Diaporama         =Diaporama;
    this->OutputFileName    =OutputFileName;
    this->FromSlide         =FromSlide;
    this->ToSlide           =ToSlide;

    //=======================================
    // Prepare container
    //=======================================

    // Alloc container
    avformat_alloc_output_context2(&Container,NULL,NULL,OutputFileName.toUtf8().constData());
    if (!Container) {
        ToLog(LOGMSG_CRITICAL,"EncodeVideo-OpenEncoder: Unable to allocate AVFormatContext");
        return false;
    }

    if (Container->oformat->flags & AVFMT_NOFILE) {
        ToLog(LOGMSG_CRITICAL,QString("EncodeVideo-OpenEncoder: Container->oformat->flags==AVFMT_NOFILE"));
        return false;
    }

    //=======================================
    // Open video stream
    //=======================================

    if (EncodeVideo) {
        // Video parameters
        this->VideoBitrate   =VideoBitrate;
        this->ImageDef       =ImageDef;
        this->VideoFrameRate =ImageDef->AVFPS;
        this->VideoCodecSubId=VideoCodecSubId;
        this->InternalWidth  =InternalWidth;
        this->InternalHeight =InternalHeight;
        this->ExtendV        =ExtendV;

        // Add stream
        if (!OpenVideoStream(&VIDEOCODECDEF[VideoCodecSubId],VideoCodecSubId,VBR,VideoFrameRate,ImageWidth,ImageHeight+ExtendV,PixelAspectRatio,VideoBitrate))
            return false;
    } else {
        // If sound only, ensure FrameRate have a value
        VideoFrameRate.num=1;
        VideoFrameRate.den=25;
    }

    //=======================================
    // Open Audio stream
    //=======================================

    if (EncodeAudio) {
        // Audio parameters
        this->AudioCodecSubId=AudioCodecSubId;

        // Add stream
        if (!OpenAudioStream(&AUDIOCODECDEF[AudioCodecSubId],AudioChannels,AudioBitrate,AudioSampleRate,Language))
            return false;

        this->AudioChannels  =AudioChannels;
        this->AudioBitrate   =AudioBitrate;
        this->AudioSampleRate=AudioSampleRate;
    }

    //********************************************
    // Open file and header
    //********************************************
    if (!PrepareTAG(Language)) return false;

    int errcode=avio_open(&Container->pb,OutputFileName.toUtf8().constData(),AVIO_FLAG_WRITE);
    if (errcode<0) {
        ToLog(LOGMSG_CRITICAL,QString("EncodeVideo-OpenEncoder: avio_open() failed : %1").arg(GetAvErrorMessage(errcode)));
        QString ErrorMessage=errcode==-2?QString("\n\n")+QApplication::translate("DlgRenderVideo","invalid path or invalid filename"):QString("\n\n%1").arg(GetAvErrorMessage(errcode));
        CustomMessageBox(ParentWindow,QMessageBox::Critical,QApplication::translate("DlgRenderVideo","Start rendering process"),
            QApplication::translate("DlgRenderVideo","Error starting encoding process:")+ErrorMessage);
        return false;
    }
    int mux_preload=int(0.5*AV_TIME_BASE);
    int mux_max_delay=int(0.7*AV_TIME_BASE);
    int mux_rate=0;
    int packet_size=-1;

    if (QString(Container->oformat->name)==QString("mpegts")) {
        packet_size =188;
        mux_rate    =int(VideoCodecCtx->bit_rate*1.1);
    } else if (QString(Container->oformat->name)==QString("matroska")) {
        mux_rate     =10080*1000;
        mux_preload  =AV_TIME_BASE/10;  // 100 ms preloading
        mux_max_delay=200*1000;         // 500 ms
    } else if (QString(Container->oformat->name)==QString("webm")) {
        mux_rate     =10080*1000;
        mux_preload  =AV_TIME_BASE/10;  // 100 ms preloading
        mux_max_delay=200*1000;         // 500 ms
    }
    Container->flags   |=AVFMT_FLAG_NONBLOCK;
    Container->max_delay=mux_max_delay;
    av_opt_set_int(Container,"preload",mux_preload,AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(Container,"muxrate",mux_rate,AV_OPT_SEARCH_CHILDREN);
    if (packet_size!=-1) Container->packet_size=packet_size;

    if (avformat_write_header(Container,NULL)<0) {
        ToLog(LOGMSG_CRITICAL,"EncodeVideo-OpenEncoder: avformat_write_header() failed");
        return false;
    }

    //********************************************
    // Log output format
    //********************************************
    av_dump_format(Container,0,OutputFileName.toUtf8().constData(),1);
    IsOpen=true;

    //=======================================
    // Init counter
    //=======================================

    dFPS    =qreal(VideoFrameRate.den)/qreal(VideoFrameRate.num);
    NbrFrame=int(qreal(Diaporama->GetPartialDuration(FromSlide,ToSlide))*dFPS/1000);    // Number of frame to generate

    return true;
}

//*************************************************************************************************************************************************
// Create a stream
//*************************************************************************************************************************************************

bool cEncodeVideo::AddStream(AVStream **Stream,const char *CodecName,AVMediaType Type) {
    const AVCodec *codec=avcodec_find_encoder_by_name(CodecName);
    AVCodecContext *codecCtx=NULL;
    if (!(codec)) {
        ToLog(LOGMSG_CRITICAL,QString("EncodeVideo-AddStream: Unable to find codec %1").arg(CodecName));
        return false;
    }
    if ((codec)->id==AV_CODEC_ID_NONE) {
        ToLog(LOGMSG_CRITICAL,QString("EncodeVideo-AddStream: codec->id==AV_CODEC_ID_NONE"));
        return false;
    }
    
    // Create stream
    *Stream=avformat_new_stream(Container,codec);
    if (!(*Stream)) {
        ToLog(LOGMSG_CRITICAL,"EncodeVideo-AddStream: avformat_new_stream() failed");
        return false;
    }
    
    // Setup encoder context for stream
    codecCtx=avcodec_alloc_context3(codec);    
 
    if (Container->oformat->flags & AVFMT_GLOBALHEADER)
        codecCtx->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    int ThreadC =((getCpuCount()/*-1*/)>1)?(getCpuCount()-1):1;
    if (ThreadC>0) codecCtx->thread_count=ThreadC;
    codecCtx->thread_type=getThreadFlags(codec->id);
    
    if (Type == AVMEDIA_TYPE_VIDEO) {
        VideoCodecCtx=codecCtx;
        VideoCodec=codec;
    } else {
        AudioCodecCtx=codecCtx;
        AudioCodec=codec;
    }

    return true;
}

//*************************************************************************************************************************************************
// Create video streams
//*************************************************************************************************************************************************

bool cEncodeVideo::OpenVideoStream(sVideoCodecDef *VideoCodecDef,int VideoCodecSubId,bool VBR,AVRational VideoFrameRate,
                                   int ImageWidth,int ImageHeight,AVRational PixelAspectRatio,int VideoBitrate) {
    if (!AddStream(&VideoStream,VideoCodecDef->ShortName,AVMEDIA_TYPE_VIDEO)) return false;

    AVDictionary *opts=NULL;
    int MinRate=-1;
    int MaxRate=-1;
    int BufSize=-1;
    int BFrames=-1;

    // Setup codec parameters
    VideoCodecCtx->width               =ImageWidth;
    VideoCodecCtx->height              =ImageHeight;
    VideoCodecCtx->pix_fmt             =AV_PIX_FMT_YUV420P;
    VideoCodecCtx->time_base           =VideoFrameRate;
    VideoStream->time_base             =VideoFrameRate;
    VideoCodecCtx->sample_aspect_ratio =PixelAspectRatio;
    VideoStream->sample_aspect_ratio   =PixelAspectRatio;
    VideoCodecCtx->gop_size            =12;

    if ((VideoCodecCtx->codec_id!=AV_CODEC_ID_H264)||(!VBR)) {
        VideoCodecCtx->bit_rate=VideoBitrate;
        av_dict_set_int(&opts,"b",VideoBitrate,0);
    }

    if (VideoCodecCtx->codec_id==AV_CODEC_ID_MPEG2VIDEO) {
        BFrames=2;
    } else if (VideoCodecCtx->codec_id==AV_CODEC_ID_MJPEG) {
        //-qscale 2 -qmin 2 -qmax 2
        VideoCodecCtx->pix_fmt             =AV_PIX_FMT_YUVJ420P;
        VideoCodecCtx->qmin                =2;
        VideoCodecCtx->qmax                =2;
        VideoCodecCtx->bit_rate_tolerance  =int(qreal(int64_t(ImageWidth)*int64_t(ImageHeight)*int64_t(VideoFrameRate.den))/qreal(VideoFrameRate.num))*10;
    } else if (VideoCodecCtx->codec_id==AV_CODEC_ID_VP8) {
        BFrames=3;
        VideoCodecCtx->gop_size=120;
        VideoCodecCtx->qmax    =ImageHeight<=576?63:51;
        av_dict_set_int(&opts,"qmax",VideoCodecCtx->qmax,0);
        VideoCodecCtx->qmin    =ImageHeight<=576?1:11;
        av_dict_set_int(&opts,"qmin",VideoCodecCtx->qmin,0);
        VideoCodecCtx->mb_lmin =VideoCodecCtx->qmin*FF_QP2LAMBDA;
        av_dict_set_int(&opts,"mb_lmin",VideoCodecCtx->mb_lmin,0);
        av_dict_set_int(&opts,"lmin",VideoCodecCtx->qmin,0);
        VideoCodecCtx->mb_lmax =VideoCodecCtx->qmax*FF_QP2LAMBDA;
        av_dict_set_int(&opts,"mb_lmax",VideoCodecCtx->mb_lmax,0);
        av_dict_set_int(&opts,"lmax",VideoCodecCtx->qmax*FF_QP2LAMBDA,0);

        if (ImageHeight<=720)
            av_dict_set_int(&opts,"profile",0,0);
        else
            av_dict_set_int(&opts,"profile",1,0);
        if (ImageHeight>576) av_dict_set_int(&opts,"slices",4,0);

        av_dict_set_int(&opts,"lag-in-frames",16,0);
        av_dict_set(&opts,"deadline","good",0);
        if (VideoCodecCtx->thread_count>0) av_dict_set_int(&opts,"cpu-used",VideoCodecCtx->thread_count,0);

    } else if (VideoCodecCtx->codec_id==AV_CODEC_ID_H264) {
        if (VideoCodecCtx->thread_count>0) av_dict_set_int(&opts,"threads",VideoCodecCtx->thread_count,0);

        switch (VideoCodecSubId) {
            case VCODEC_H264HQ:     // High Quality H.264 AVC/MPEG-4 AVC
            case VCODEC_H264PQ:     // Phone Quality H.264 AVC/MPEG-4 AVC
                av_dict_set_int(&opts,"refs",3,0);
                if (VBR) {
                    MinRate=int(double(VideoBitrate)*VBRMINCOEF);
                    MaxRate=int(double(VideoBitrate)*VBRMAXCOEF);
                    BufSize=int(double(VideoBitrate)*4);
                }
                av_dict_set(&opts,"preset",(VideoCodecSubId==VCODEC_H264HQ?Diaporama->ApplicationConfig->Preset_HQ:Diaporama->ApplicationConfig->Preset_PQ).toLocal8Bit(),0);
                av_dict_set(&opts,"profile",(VideoCodecSubId==VCODEC_H264HQ?Diaporama->ApplicationConfig->Profile_HQ:Diaporama->ApplicationConfig->Profile_PQ).toLocal8Bit(),0);
                av_dict_set(&opts,"tune",(VideoCodecSubId==VCODEC_H264HQ?Diaporama->ApplicationConfig->Tune_HQ:Diaporama->ApplicationConfig->Tune_PQ).toLocal8Bit(),0);
                break;

            case VCODEC_X264LL: // x264 lossless
                av_dict_set(&opts,"preset","veryfast",0);
                av_dict_set_int(&opts,"refs",3,0);
                av_dict_set_int(&opts,"qp",0,0);
                break;
        }

    }

    VideoCodecCtx->keyint_min=1;
    av_dict_set_int(&opts,"g",VideoCodecCtx->gop_size,0);
    av_dict_set_int(&opts,"keyint_min",1,0);

    if (MinRate!=-1) {
        av_dict_set_int(&opts,"minrate",MinRate,0);
        av_dict_set_int(&opts,"maxrate",MaxRate,0);
        av_dict_set_int(&opts,"bufsize",BufSize,0);
    }

    if (BFrames!=-1) {
        VideoCodecCtx->max_b_frames=BFrames;
        av_dict_set_int(&opts,"bf",BFrames,0);
    }
    VideoCodecCtx->has_b_frames=(VideoCodecCtx->max_b_frames>0)?1:0;

    // Open encoder
    int errcode=avcodec_open2(VideoCodecCtx,VideoCodec,&opts);
    if (errcode<0) {
        char Buf[2048];
        av_strerror(errcode,Buf,2048);
        ToLog(LOGMSG_CRITICAL,"EncodeVideo-OpenVideoStream: avcodec_open2() failed: "+QString(Buf));
        return false;
    }
    
    errcode = avcodec_parameters_from_context(VideoStream->codecpar, VideoCodecCtx);
    if (errcode < 0) {
        ToLog(LOGMSG_CRITICAL, "Failed to copy video encoder parameters to output stream\n");
        return false;
    }

    // Create VideoFrameConverter
    VideoFrameConverter=sws_getContext(
        InternalWidth,VideoCodecCtx->height,PIXFMT,                          // Src Widht,Height,Format
        VideoCodecCtx->width,VideoCodecCtx->height,VideoCodecCtx->pix_fmt,   // Destination Width,Height,Format
        SWS_BICUBIC,                                                         // flags
        NULL,NULL,NULL);                                                     // src Filter,dst Filter,param
    if (!VideoFrameConverter) {
        ToLog(LOGMSG_CRITICAL,"EncodeVideo-OpenVideoStream: sws_getContext() failed");
        return false;
    }

    // Create and prepare VideoFrame and VideoFrameBuf
    VideoFrame=av_frame_alloc();  // Allocate structure for RGB image
    if (!VideoFrame) {
        ToLog(LOGMSG_CRITICAL,"EncodeVideo-OpenVideoStream: avcodec_alloc_frame() failed");
        return false;
    } else {
        VideoFrameBufSize=av_image_get_buffer_size(VideoCodecCtx->pix_fmt,VideoCodecCtx->width,VideoCodecCtx->height,1);
        VideoFrameBuf    =(uint8_t *)av_malloc(VideoFrameBufSize);
        if ((!VideoFrameBufSize)||(!VideoFrameBuf)) {
            ToLog(LOGMSG_CRITICAL,"EncodeVideo-OpenVideoStream: av_malloc() failed for VideoFrameBuf");
            return false;
        }
    }

    return true;
}

//*************************************************************************************************************************************************
// Create audio streams
//*************************************************************************************************************************************************

bool cEncodeVideo::OpenAudioStream(sAudioCodecDef *AudioCodecDef,int &AudioChannels,int &AudioBitrate,int &AudioSampleRate,QString Language) {
    if (!AddStream(&AudioStream,AudioCodecDef->ShortName,AVMEDIA_TYPE_AUDIO)) return false;

    AVDictionary    *opts   =NULL;

    // Setup codec parameters
    AudioCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
    av_channel_layout_default(&AudioCodecCtx->ch_layout, AudioChannels);
    AudioCodecCtx->sample_rate = AudioSampleRate;

    av_dict_set(&AudioStream->metadata,"language",Language.toUtf8().constData(),0);

    if (AudioCodecCtx->codec_id==AV_CODEC_ID_PCM_S16LE) {
        AudioBitrate=AudioSampleRate*16*AudioChannels;
    } else if (AudioCodecCtx->codec_id==AV_CODEC_ID_FLAC) {
        av_dict_set_int(&opts,"lpc_coeff_precision",15,0);
        av_dict_set_int(&opts,"lpc_type",2,0);
        av_dict_set_int(&opts,"lpc_passes",1,0);
        av_dict_set_int(&opts,"min_partition_order",0,0);
        av_dict_set_int(&opts,"max_partition_order",8,0);
        av_dict_set_int(&opts,"prediction_order_method",0,0);
        av_dict_set_int(&opts,"ch_mode",-1,0);
    } else if (AudioCodecCtx->codec_id==AV_CODEC_ID_AAC) {
        if (QString(AUDIOCODECDEF[2].ShortName)=="aac") {
            AudioCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
            AudioCodecCtx->sample_fmt =AV_SAMPLE_FMT_FLTP;
        }
    } else if (AudioCodecCtx->codec_id==AV_CODEC_ID_MP2) {

    } else if (AudioCodecCtx->codec_id==AV_CODEC_ID_MP3) {
        AudioCodecCtx->sample_fmt =AV_SAMPLE_FMT_S16P;
        av_dict_set_int(&opts,"reservoir",1,0);
    } else if (AudioCodecCtx->codec_id==AV_CODEC_ID_VORBIS) {
        AudioCodecCtx->sample_fmt =AV_SAMPLE_FMT_FLTP;
    } else if (AudioCodecCtx->codec_id==AV_CODEC_ID_AC3) {
        AudioCodecCtx->sample_fmt =AV_SAMPLE_FMT_FLTP;
    } else if ((AudioCodecCtx->codec_id==AV_CODEC_ID_AMR_NB) || (AudioCodecCtx->codec_id==AV_CODEC_ID_AMR_WB)) {
      AudioCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_MONO;  
      AudioChannels = 1;
    }
    AudioCodecCtx->bit_rate    =AudioBitrate;
    av_dict_set_int(&opts,"ab",AudioBitrate,0);

    int errcode=avcodec_open2(AudioCodecCtx,AudioCodec,&opts);
    if (errcode<0) {
        char Buf[2048];
        av_strerror(errcode,Buf,2048);
        ToLog(LOGMSG_CRITICAL,"EncodeVideo-OpenAudioStream: avcodec_open2() failed: "+QString(Buf));
        return false;
    }
    
    errcode = avcodec_parameters_from_context(AudioStream->codecpar, AudioCodecCtx);
    if (errcode < 0) {
        ToLog(LOGMSG_CRITICAL, "Failed to copy audio encoder parameters to output stream\n");
        return false;
    }

    AudioFrame=av_frame_alloc();  // Allocate structure for RGB image
    if (AudioFrame==NULL) {
        ToLog(LOGMSG_CRITICAL,QString("EncodeVideo-OpenAudioStream:: avcodec_alloc_frame failed"));
        return false;
    }

    return true;
}

//*************************************************************************************************************************************************

bool cEncodeVideo::PrepareTAG(QString Language) {
    // Set TAGS
    av_dict_set(&Container->metadata,"language",Language.toUtf8().constData(),0);
    av_dict_set(&Container->metadata,"title",AdjustMETA(Diaporama->ProjectInfo->Title==""?QFileInfo(OutputFileName).baseName():Diaporama->ProjectInfo->Title).toUtf8().constData(),0);
    av_dict_set(&Container->metadata,"artist",AdjustMETA(Diaporama->ProjectInfo->Author).toUtf8().constData(),0);
    av_dict_set(&Container->metadata,"album",AdjustMETA(Diaporama->ProjectInfo->Album).toUtf8().constData(),0);
    av_dict_set(&Container->metadata,"comment",AdjustMETA(Diaporama->ProjectInfo->Comment).toUtf8().constData(),0);
    av_dict_set(&Container->metadata,"date",QString("%1").arg(Diaporama->ProjectInfo->EventDate.year()).toUtf8().constData(),0);
    av_dict_set(&Container->metadata,"composer",QString(QString(APPLICATION_NAME)+QString(" ")+CurrentAppName).toUtf8().constData(),0);
    av_dict_set(&Container->metadata,"creation_time",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toUtf8().constData(),0); // ISO 8601 format

    // Set Chapters (only if video stream)
    if (VideoStream) {
        for (int i=FromSlide;i<=ToSlide;i++) if ((i==FromSlide)||(Diaporama->List[i]->StartNewChapter)) {
            AVChapter *Chapter=(AVChapter *)av_mallocz(sizeof(AVChapter));
            int64_t   Start   =Diaporama->GetObjectStartPosition(i)+(i>FromSlide?Diaporama->List[i]->GetTransitDuration():0)-Diaporama->GetObjectStartPosition(FromSlide);
            int64_t   Duration=Diaporama->List[i]->GetDuration()-(i>FromSlide?Diaporama->List[i]->GetTransitDuration():0);
            int       NextC   =i+1;
            while ((NextC<ToSlide)&&(!Diaporama->List[NextC]->StartNewChapter)) {
                Duration=Duration+Diaporama->List[NextC]->GetDuration();
                NextC++;
                if (NextC<=ToSlide) Duration=Duration-Diaporama->List[NextC-1]->GetTransitDuration();
            }
            int64_t   End     =Start+Duration;
            int64_t   ts_off  =av_rescale_q(Container->start_time,AV_TIME_BASE_Q,VideoStream->time_base);
            Chapter->id       =Container->nb_chapters;
            Chapter->time_base=VideoStream->time_base;
            Chapter->start    =av_rescale_q((Start-ts_off)*1000,AV_TIME_BASE_Q,VideoStream->time_base);
            Chapter->end      =av_rescale_q((End-ts_off)*1000,AV_TIME_BASE_Q,VideoStream->time_base);
            QString CptName=Diaporama->List[i]->StartNewChapter?Diaporama->List[i]->ChapterName:Diaporama->ProjectInfo->Title;
            av_dict_set(&Chapter->metadata,"title",CptName.toUtf8(),0);
            Container->chapters=(AVChapter **)av_realloc(Container->chapters,sizeof(AVChapter)*(Container->nb_chapters+1));
            Container->chapters[Container->nb_chapters]=Chapter;
            Container->nb_chapters++;
        }
    }

    return true;
}

//*************************************************************************************************************************************************

QString cEncodeVideo::AdjustMETA(QString Text) {
    //Metadata keys or values containing special characters (’=’, ’;’, ’#’, ’\’ and a newline) must be escaped with a backslash ’\’.
    Text.replace("=","\\=");
    Text.replace(";","\\;");
    Text.replace("#","\\#");
    //Text.replace("\\","\\\\");
    Text.replace("\n","\\\n");
    return Text;
}

//*************************************************************************************************************************************************

bool cEncodeVideo::DoEncode() {
    bool                    Continue=true;
    cSoundBlockList         RenderMusic,ToEncodeMusic;
    cDiaporamaObjectInfo    *PreviousFrame=NULL,*PreviousPreviousFrame=NULL;
    cDiaporamaObjectInfo    *Frame        =NULL;
    int                     FrameSize     =0;
    QObject                 FakeParentObject(NULL);

    IncreasingVideoPts=qreal(1000)/dFPS;

    // Init RenderMusic and ToEncodeMusic
    if (AudioStream) {
        RenderMusic.SetFPS(IncreasingVideoPts,AudioChannels,AudioSampleRate,AV_SAMPLE_FMT_S16);
        FrameSize=AudioCodecCtx->frame_size;
        if ((!FrameSize)&&(AudioCodecCtx->codec_id==AV_CODEC_ID_PCM_S16LE)) FrameSize=1024;
        if ((FrameSize==0)&&(VideoStream)) FrameSize=(AudioCodecCtx->sample_rate*AudioStream->time_base.num)/AudioStream->time_base.den;
        int ComputedFrameSize=AudioChannels*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)*FrameSize;
        if (ComputedFrameSize==0) ComputedFrameSize=RenderMusic.SoundPacketSize;
        ToEncodeMusic.SetFrameSize(ComputedFrameSize,RenderMusic.Channels,AudioSampleRate,AV_SAMPLE_FMT_S16);
    }

    AudioFrameNbr       =0;
    VideoFrameNbr       =0;
    LastAudioPts        =0;
    LastVideoPts        =0;
    IncreasingAudioPts  =AudioStream?FrameSize*1000*AudioCodecCtx->time_base.num/AudioCodecCtx->time_base.den:0;
    StartTime           =QTime::currentTime();
    LastCheckTime       =StartTime;                                     // Display control : last time the loop start
    Position            =Diaporama->GetObjectStartPosition(FromSlide);  // Render current position
    ColumnStart         =-1;                                            // Render start position of current object
    Column              =FromSlide-1;                                   // Render current object
    RenderedFrame       =0;

    // Init Resampler (if needed)
    if (AudioStream) {
        if ((AudioCodecCtx->sample_fmt!=RenderMusic.SampleFormat)||(AudioCodecCtx->ch_layout.nb_channels!=RenderMusic.Channels)||(AudioSampleRate!=RenderMusic.SamplingRate)) {
            if (!AudioResampler) {
                AudioResampler=swr_alloc();
                AVChannelLayout in_channel_layout;
                av_channel_layout_default(&in_channel_layout, ToEncodeMusic.Channels);
                av_opt_set_chlayout(AudioResampler,"in_chlayout",&in_channel_layout,0);
                av_opt_set_int(AudioResampler,"in_sample_rate",ToEncodeMusic.SamplingRate,0);
                av_opt_set_chlayout(AudioResampler,"out_chlayout",&AudioCodecCtx->ch_layout,0);
                av_opt_set_int(AudioResampler,"out_sample_rate",AudioCodecCtx->sample_rate,0);
                av_opt_set_int(AudioResampler,"in_channel_count",ToEncodeMusic.Channels,0);
                av_opt_set_int(AudioResampler,"out_channel_count",AudioCodecCtx->ch_layout.nb_channels,0);
                av_opt_set_sample_fmt(AudioResampler,"out_sample_fmt",AudioCodecCtx->sample_fmt,0);
                av_opt_set_sample_fmt(AudioResampler,"in_sample_fmt",ToEncodeMusic.SampleFormat,0);
                if (swr_init(AudioResampler)<0) {
                    ToLog(LOGMSG_CRITICAL,QString("DoEncode: swr_alloc_set_opts failed"));
                    Continue=false;
                }
            }
        }
    }

    // Prepare a DefaultSourceImage to not create it for each frame
    QImage DefaultSourceImage;
    if ((InternalWidth!=0)&&(InternalHeight!=0)) {
        // create an empty transparent image
        DefaultSourceImage=QImage(InternalWidth,InternalHeight,QImage::Format_ARGB32_Premultiplied);
        QPainter PT;
        PT.begin(&DefaultSourceImage);
        PT.setCompositionMode(QPainter::CompositionMode_Source);
        PT.fillRect(QRect(0,0,InternalWidth,InternalHeight),Qt::transparent);
        PT.setCompositionMode(QPainter::CompositionMode_SourceOver);
        PT.end();
    } else {
        // Create a very small image to have a ptr
        DefaultSourceImage=QImage(5,5,QImage::Format_ARGB32_Premultiplied);
    }

    // Define InterleaveFrame to not compute it for each frame
    InterleaveFrame=(strcmp(Container->oformat->name,"avi")!=0);
    
    for (RenderedFrame=0;Continue && (RenderedFrame<NbrFrame);RenderedFrame++) {
        // Calculate position & column
        AdjustedDuration=((Column>=0)&&(Column<Diaporama->List.count()))?Diaporama->List[Column]->GetDuration()-Diaporama->GetTransitionDuration(Column+1):0;
        if (AdjustedDuration<33) AdjustedDuration=33; // Not less than 1/30 sec

        if ((ColumnStart==-1)||(Column==-1)||((Column<Diaporama->List.count())&&((ColumnStart+AdjustedDuration)<=Position))) {
            Column++;
            AdjustedDuration=((Column>=0)&&(Column<Diaporama->List.count()))?Diaporama->List[Column]->GetDuration()-Diaporama->GetTransitionDuration(Column+1):0;
            if (AdjustedDuration<33) AdjustedDuration=33; // Not less than 1/30 sec
            ColumnStart=Diaporama->GetObjectStartPosition(Column);
            Diaporama->CloseUnusedFFMPEG(Column);
            if (LastCheckTime.msecsTo(QTime::currentTime())>=1000) {
                LastCheckTime=QTime::currentTime();
            }
        }

        // Get current frame
        Frame=new cDiaporamaObjectInfo(PreviousFrame,Position,Diaporama,IncreasingVideoPts,AudioStream!=NULL);
        // Ensure MusicTracks are ready
        if ((AudioStream)&&(Frame->CurrentObject)&&(Frame->CurrentObject_MusicTrack==NULL)) {
            Frame->CurrentObject_MusicTrack=new cSoundBlockList();
            Frame->CurrentObject_MusicTrack->SetFPS(IncreasingVideoPts,AudioChannels,AudioSampleRate,AV_SAMPLE_FMT_S16);
        }
        if ((AudioStream)&&(Frame->TransitObject)&&(Frame->TransitObject_MusicTrack==NULL)&&(Frame->TransitObject_MusicObject!=NULL)&&(Frame->TransitObject_MusicObject!=Frame->CurrentObject_MusicObject)) {
            Frame->TransitObject_MusicTrack=new cSoundBlockList();
            Frame->TransitObject_MusicTrack->SetFPS(IncreasingVideoPts,AudioChannels,AudioSampleRate,AV_SAMPLE_FMT_S16);
        }

        // Ensure SoundTracks are ready
        if ((AudioStream)&&(Frame->CurrentObject)&&(Frame->CurrentObject_SoundTrackMontage==NULL)) {
            Frame->CurrentObject_SoundTrackMontage=new cSoundBlockList();
            Frame->CurrentObject_SoundTrackMontage->SetFPS(IncreasingVideoPts,AudioChannels,AudioSampleRate,AV_SAMPLE_FMT_S16);
        }
        if ((AudioStream)&&(Frame->TransitObject)&&(Frame->TransitObject_SoundTrackMontage==NULL)) {
            Frame->TransitObject_SoundTrackMontage=new cSoundBlockList();
            Frame->TransitObject_SoundTrackMontage->SetFPS(IncreasingVideoPts,AudioChannels,AudioSampleRate,AV_SAMPLE_FMT_S16);
        }

        // Prepare frame (if W=H=0 then soundonly)
        if ((Frame->IsTransition)&&(Frame->TransitObject)) Diaporama->CreateObjectContextList(Frame,InternalWidth,InternalHeight,false,false,true,PreparedTransitBrushList,&FakeParentObject);
        Diaporama->CreateObjectContextList(Frame,InternalWidth,InternalHeight,true,false,true,PreparedBrushList,&FakeParentObject);
        Diaporama->LoadSources(Frame,InternalWidth,InternalHeight,false,true,PreparedTransitBrushList,PreparedBrushList,2);

        // Ensure previous Assembly was ended
        if (ThreadAssembly.isRunning()) ThreadAssembly.waitForFinished();

        // mix audio data
        if (Frame->CurrentObject_MusicTrack) {
            int MaxJ=Frame->CurrentObject_MusicTrack->NbrPacketForFPS;
            //if (MaxJ>Frame->CurrentObject_MusicTrack->ListCount()) MaxJ=Frame->CurrentObject_MusicTrack->ListCount();
            RenderMusic.Mutex.lock();
            for (int j=0;j<MaxJ;j++) {
                int16_t *Music=(((Frame->IsTransition)&&(Frame->TransitObject)&&(!Frame->TransitObject->MusicPause))||
                                (!Frame->CurrentObject->MusicPause))?Frame->CurrentObject_MusicTrack->DetachFirstPacket(true):NULL;
                int16_t *Sound=(Frame->CurrentObject_SoundTrackMontage!=NULL)?Frame->CurrentObject_SoundTrackMontage->DetachFirstPacket(true):NULL;
                RenderMusic.MixAppendPacket(Frame->CurrentObject_StartTime+Frame->CurrentObject_InObjectTime,Music,Sound,true);
            }
            RenderMusic.Mutex.unlock();
        }
        // Delete PreviousFrame used by assembly thread
        if (PreviousPreviousFrame) delete PreviousPreviousFrame;

        // Keep actual PreviousFrame for next time
        PreviousPreviousFrame=PreviousFrame;

        // If not static image then compute using threaded function
        if ((!PreviousFrame)||(PreviousFrame->RenderedImage.isNull()))
            ThreadAssembly.setFuture(QtConcurrent::run(this,&cEncodeVideo::Assembly,Frame,PreviousFrame,&RenderMusic,&ToEncodeMusic,Continue));
        else
            Assembly(Frame,PreviousFrame,&RenderMusic,&ToEncodeMusic,Continue);

        // Calculate next position
        if (Continue) {
            Position+=IncreasingVideoPts;
            PreviousFrame=Frame;
            Frame=NULL;
        }

        // Stop the process if error occur or user ask to stop
        Continue=Continue && !StopProcessWanted;
    }

    if (ThreadAssembly.isRunning())    ThreadAssembly.waitForFinished();
    if (ThreadEncodeAudio.isRunning()) ThreadEncodeAudio.waitForFinished();
    if (ThreadEncodeVideo.isRunning()) ThreadEncodeVideo.waitForFinished();

    Position=-1;
    ColumnStart=0;
    AdjustedDuration=0;

    // Cleaning
    if (PreviousPreviousFrame)  delete PreviousPreviousFrame;
    if (PreviousFrame!=NULL)    delete PreviousFrame;
    if (Frame!=NULL)            delete Frame;

    CloseEncoder();

    return Continue;
}

//*************************************************************************************************************************************************

void cEncodeVideo::Assembly(cDiaporamaObjectInfo *Frame,cDiaporamaObjectInfo *PreviousFrame,cSoundBlockList *RenderMusic,cSoundBlockList *ToEncodeMusic,bool &Continue) {
    // Make final assembly
    Diaporama->DoAssembly(ComputePCT(Frame->CurrentObject?Frame->CurrentObject->GetSpeedWave():0,Frame->TransitionPCTDone),Frame,InternalWidth,InternalHeight,QTPIXFMT);

    // Ensure previous threaded encoding was ended before continuing
    if (ThreadEncodeAudio.isRunning()) ThreadEncodeAudio.waitForFinished();
    if (ThreadEncodeVideo.isRunning()) ThreadEncodeVideo.waitForFinished();

    // Write data to disk
    if ((Continue)&&(AudioStream)&&(AudioFrame))
        ThreadEncodeAudio.setFuture(QtConcurrent::run(this,&cEncodeVideo::EncodeMusic,Frame,RenderMusic,ToEncodeMusic,Continue));

    if ((Continue)&&(VideoStream)&&(VideoFrameConverter)&&(VideoFrame)) {
        QImage *Image=((PreviousFrame)&&(!PreviousFrame->IsTransition)&&(Frame->IsShotStatic)&&(!Frame->IsTransition))?NULL:&Frame->RenderedImage;
        ThreadEncodeVideo.setFuture(QtConcurrent::run(this,&cEncodeVideo::EncodeVideo,Image,Continue));
    }
}

//*************************************************************************************************************************************************

void cEncodeVideo::EncodeMusic(cDiaporamaObjectInfo *Frame,cSoundBlockList *RenderMusic,cSoundBlockList *ToEncodeMusic,bool &Continue) {
    // Transfert RenderMusic data to EncodeMusic data
    int MaxPQ=RenderMusic->NbrPacketForFPS;
    if (MaxPQ>RenderMusic->ListCount()) MaxPQ=RenderMusic->ListCount();
    for (int PQ=0;(Continue)&&(PQ<MaxPQ);PQ++) {
        uint8_t *PacketSound=(uint8_t *)RenderMusic->DetachFirstPacket();
        if (PacketSound==NULL) {
            PacketSound=(uint8_t *)av_malloc(RenderMusic->SoundPacketSize+8);
            memset(PacketSound,0,RenderMusic->SoundPacketSize);
        }
        ToEncodeMusic->AppendData(Frame->CurrentObject_StartTime+Frame->CurrentObject_InObjectTime,(int16_t *)PacketSound,RenderMusic->SoundPacketSize);
        av_free(PacketSound);
    }

    int         errcode;
    int64_t     DestNbrSamples=ToEncodeMusic->SoundPacketSize/(ToEncodeMusic->Channels*av_get_bytes_per_sample(ToEncodeMusic->SampleFormat));
    int         DestSampleSize=AudioCodecCtx->ch_layout.nb_channels*av_get_bytes_per_sample(AudioCodecCtx->sample_fmt);
    uint8_t    *DestPacket   =NULL;
    int16_t     *PacketSound  =NULL;
    int64_t     DestPacketSize=DestNbrSamples*DestSampleSize;

    // Flush audio frame of ToEncodeMusic
    while ((Continue)&&(ToEncodeMusic->ListCount()>0)&&(!StopProcessWanted)) {
        PacketSound=ToEncodeMusic->DetachFirstPacket();
        if (PacketSound==NULL) {
            ToLog(LOGMSG_CRITICAL,QString("EncodeMusic: PacketSound==NULL"));
            Continue=false;
        } else {
            if (AudioResampler!=NULL) {
                int out_samples=av_rescale_rnd(swr_get_delay(AudioResampler,ToEncodeMusic->SamplingRate)+DestNbrSamples,AudioCodecCtx->sample_rate,ToEncodeMusic->SamplingRate,AV_ROUND_UP);
                av_samples_alloc(&AudioResamplerBuffer,NULL,AudioCodecCtx->ch_layout.nb_channels,out_samples,AudioCodecCtx->sample_fmt,0);
                if (!AudioResamplerBuffer) {
                    ToLog(LOGMSG_CRITICAL,QString("EncodeMusic: AudioResamplerBuffer allocation failed"));
                    Continue=false;
                }
                uint8_t *in_data[AVRESAMPLE_MAX_CHANNELS]={0},*out_data[AVRESAMPLE_MAX_CHANNELS]={0};
                int     in_linesize=0,out_linesize=0;
                if (av_samples_fill_arrays(in_data,&in_linesize,(uint8_t *)PacketSound,ToEncodeMusic->Channels,DestNbrSamples,ToEncodeMusic->SampleFormat,0)<0) {
                    ToLog(LOGMSG_CRITICAL,QString("failed in_data fill arrays"));
                    Continue=false;
                } else {
                    if (av_samples_fill_arrays(out_data,&out_linesize,AudioResamplerBuffer,AudioCodecCtx->ch_layout.nb_channels,out_samples,AudioCodecCtx->sample_fmt,0)<0) {
                        ToLog(LOGMSG_CRITICAL,QString("failed out_data fill arrays"));
                        Continue=false;
                    } else {
                        DestPacketSize=swr_convert(AudioResampler,out_data,out_samples,(const uint8_t **)in_data,DestNbrSamples)*DestSampleSize;
                        if (DestPacketSize<=0) {
                            ToLog(LOGMSG_CRITICAL,QString("EncodeMusic: swr_convert failed"));
                            Continue=false;
                        }
                        DestPacket=(uint8_t *)out_data[0];
                    }
                }
            } else DestPacket=(uint8_t *)PacketSound;

            if (Continue) {
                // Init AudioFrame
                AVRational AVR;

                av_frame_unref(AudioFrame);

                AVR.num                     =1;
                AVR.den                     =AudioCodecCtx->sample_rate;
                AudioFrame->nb_samples      =DestPacketSize/DestSampleSize;
                AudioFrame->pts             =av_rescale_q(AudioFrame->nb_samples*AudioFrameNbr,AVR,AudioStream->time_base);
                AudioFrame->format          =AudioCodecCtx->sample_fmt;
                av_channel_layout_copy(&AudioFrame->ch_layout, &AudioCodecCtx->ch_layout);
                // fill buffer
                errcode=avcodec_fill_audio_frame(AudioFrame,AudioCodecCtx->ch_layout.nb_channels,AudioCodecCtx->sample_fmt,(const uint8_t*)DestPacket,DestPacketSize,1);
                if (errcode>=0) {
                    // Init packet
                    AVPacket *pkt = av_packet_alloc();
                    pkt->size=0;
                    pkt->data=NULL;

                    errcode=avcodec_send_frame(AudioCodecCtx,AudioFrame);
                    if (errcode<0) {
                        char Buf[2048];
                        av_strerror(errcode,Buf,2048);
                        ToLog(LOGMSG_CRITICAL,QString("EncodeMusic: avcodec_send_frame failed: ")+QString(Buf));
                        Continue=false;
                    }
                    errcode=avcodec_receive_packet(AudioCodecCtx, pkt);
                    if (errcode==0) {
                        pkt->stream_index=AudioStream->index;

                        // Encode frame
                        Mutex.lock();
                        
                        if (InterleaveFrame) {
                            errcode=av_interleaved_write_frame(Container,pkt);
                        } else {
                            pkt->pts=AV_NOPTS_VALUE;
                            pkt->dts=AV_NOPTS_VALUE;
                            errcode=av_write_frame(Container,pkt);
                        }
                        
                        Mutex.unlock();

                        if (errcode!=0) {
                            char Buf[2048];
                            av_strerror(errcode,Buf,2048);
                            ToLog(LOGMSG_CRITICAL,QString("EncodeMusic: write_frame failed: ")+QString(Buf));
                            Continue=false;
                        }
                    }
                    LastAudioPts+=IncreasingAudioPts;
                    AudioFrameNbr++;
                    av_packet_free(&pkt);
               } else {
                    char Buf[2048];
                    av_strerror(errcode,Buf,2048);
                    ToLog(LOGMSG_CRITICAL,QString("EncodeMusic: avcodec_fill_audio_frame() failed: ")+QString(Buf));
                    Continue=false;
               }
            }

            if ((AudioFrame->extended_data)&&(AudioFrame->extended_data!=AudioFrame->data)) {
                av_free(AudioFrame->extended_data);
                AudioFrame->extended_data=NULL;
            }

        }
        av_free(PacketSound);
        av_free(AudioResamplerBuffer);
        AudioResamplerBuffer=NULL;
    }

}

//*************************************************************************************************************************************************

void cEncodeVideo::EncodeVideo(QImage *SrcImage,bool &Continue) {
    QImage *Image=SrcImage;
    int     errcode;

    if (Image) {
        av_frame_unref(VideoFrame);
        
        if (av_image_fill_arrays(
            VideoFrame->data,              // Frame to prepare
            VideoFrame->linesize,
            VideoFrameBuf,                 // Buffer which will contain the image data
            VideoCodecCtx->pix_fmt,        // The format in which the picture data is stored
            VideoCodecCtx->width,          // The width of the image in pixels
            VideoCodecCtx->height,         // The height of the image in pixels
            1
        )!=VideoFrameBufSize) {
            ToLog(LOGMSG_CRITICAL,"EncodeVideo-EncodeVideo: avpicture_fill() failed for VideoFrameBuf");
            return;
        }
        // Apply ExtendV
        if ((Continue)&&(!StopProcessWanted)&&(Image->height()!=VideoCodecCtx->height)) {
            Image=new QImage(InternalWidth,VideoCodecCtx->height,QTPIXFMT);
            QPainter P;
            P.begin(Image);
            P.fillRect(QRect(0,0,Image->width(),Image->height()),Qt::black);
            P.drawImage(0,(VideoCodecCtx->height-SrcImage->height())/2,*SrcImage);
            P.end();
        }

        // Now, convert image
        if ((Continue)&&(!StopProcessWanted)) {
            uint8_t *data={(uint8_t *)Image->bits()};
            int LineSize=Image->bytesPerLine();
            int Ret=sws_scale(
                VideoFrameConverter,    // libswscale converter
                &data,                  // Source buffer
                &LineSize,              // Source Stride ?
                0,                      // Source SliceY:the position in the source image of the slice to process, that is the number (counted starting from zero) in the image of the first row of the slice
                Image->height(),        // Source SliceH:the height of the source slice, that is the number of rows in the slice
                VideoFrame->data,       // Destination buffer
                VideoFrame->linesize    // Destination Stride
            );
            if (Ret!=Image->height()) {
                ToLog(LOGMSG_CRITICAL,"EncodeVideo-ConvertRGBToYUV: sws_scale() failed");
                Continue=false;
            }
        }
    }

    if ((VideoFrameNbr%VideoCodecCtx->gop_size)==0)
        VideoFrame->pict_type=AV_PICTURE_TYPE_I;
    else
        VideoFrame->pict_type=(AVPictureType)0;
    VideoFrame->pts=VideoFrameNbr;
    VideoFrame->format=VideoCodecCtx->pix_fmt;
    VideoFrame->width=VideoCodecCtx->width;
    VideoFrame->height=VideoCodecCtx->height;


    if ((Continue)&&(!StopProcessWanted)) {

        AVPacket *pkt = av_packet_alloc();
        pkt->size=0;
        pkt->data=NULL;

        errcode=avcodec_send_frame(VideoCodecCtx,VideoFrame);
        if (errcode<0) {
            char Buf[2048];
            av_strerror(errcode,Buf,2048);
            ToLog(LOGMSG_CRITICAL,QString("EncodeVideo: avcodec_send_frame failed")+QString(Buf));
            Continue=false;
        }
        errcode=avcodec_receive_packet(VideoCodecCtx, pkt);
        if (errcode==0) {
            if (pkt->dts!=AV_NOPTS_VALUE) pkt->dts=av_rescale_q(pkt->dts,VideoCodecCtx->time_base,VideoStream->time_base);
            if (pkt->pts!=AV_NOPTS_VALUE) pkt->pts=av_rescale_q(pkt->pts,VideoCodecCtx->time_base,VideoStream->time_base);
            pkt->duration=av_rescale_q(pkt->duration,VideoCodecCtx->time_base,VideoStream->time_base);
            pkt->stream_index=VideoStream->index;
            
            // Encode frame
            Mutex.lock();
            
            if (InterleaveFrame) {
                errcode=av_interleaved_write_frame(Container,pkt);
            } else {
                pkt->dts=AV_NOPTS_VALUE;
                pkt->pts=AV_NOPTS_VALUE;
                errcode=av_write_frame(Container,pkt);
            }
            Mutex.unlock();

            if (errcode!=0) {
                char Buf[2048];
                av_strerror(errcode,Buf,2048);
                ToLog(LOGMSG_CRITICAL,QString("EncodeVideo: write_frame failed: ")+QString(Buf));
                Continue=false;
            }
        }

        LastVideoPts+=IncreasingVideoPts;
        VideoFrameNbr++;
        av_packet_free(&pkt);

        if ((VideoFrame->extended_data)&&(VideoFrame->extended_data!=VideoFrame->data)) {
            av_free(VideoFrame->extended_data);
            VideoFrame->extended_data=NULL;
        }
    }
    if ((Image)&&(Image!=SrcImage)) delete Image;
}

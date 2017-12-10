#include <iostream>
#include <cstdio>

#ifdef __cplusplus
extern "C"{
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libavdevice/avdevice.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
#endif

#ifdef __cplusplus
};
#endif

char *input_name = "video4linux2";
char *file_name = "/dev/video0";
char *out_file = "yuv420.yuv";
  
int captureOneFrame(){
  
  AVFormatContext *fmtCtx = NULL;
  AVInputFormat *inputFmt;
  AVPacket *packet;
  AVCodecContext *pCodecCtx;
  struct SwsContext *sws_ctx;
  
  int i;
  int ret;
  int videoindex;
  
  enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_YUVJ420P;
  const char *dst_size = NULL;
  const char *src_size = NULL;
  uint8_t *src_data[4];
  uint8_t *dst_data[4];
  int src_linesize[4];
  int dst_linesize[4];
  int src_bufsize;
  int dst_bufsize;
  
  int src_w;
  int src_h;/*
  int dst_w = 1280;
  int dst_h = 720;*/
  
  int dst_w = 640;
  int dst_h = 360;
  
  av_log_set_level(AV_LOG_DEBUG);
  
  inputFmt = av_find_input_format(input_name);
  if (inputFmt == NULL){
    std::cout<<"can not find input format. "<<std::endl;
    return -1;
  }
  
  const char *videosize1 = "640x360";
  AVDictionary *option;
  av_dict_set(&option, "video_size", videosize1, 0);
  
  if (avformat_open_input(&fmtCtx, file_name, inputFmt, &option)<0){
    std::cout<<"can not open input file."<<std::endl;
    return -1;
  }
  
//   av_dump_format(fmtCtx, 0, NULL, 0);
  
  videoindex = -1;
  for (i=0;i<fmtCtx->nb_streams;i++){
    if(fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
      videoindex = i;
      break;
    }
  }
  
  if(videoindex == -1){
    std::cout<<"find video stream failed, now return."<<std::endl;
    return -1;
  }
  
  pCodecCtx = fmtCtx->streams[videoindex]->codec;
  
  std::cout<<"picture width height format"<<pCodecCtx->width<<pCodecCtx->height<<pCodecCtx->pix_fmt<<std::endl;
  
  std::cout<<"origin pix_fmt "<<pCodecCtx->pix_fmt<<std::endl;
  sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt
    ,dst_w, dst_h, dst_pix_fmt, SWS_BILINEAR, NULL, NULL, NULL);
  
  src_bufsize = av_image_alloc(src_data, src_linesize, pCodecCtx->width, pCodecCtx->height
    , pCodecCtx->pix_fmt, 16);
  
  dst_bufsize = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pix_fmt, 16);
  
  packet = (AVPacket *)av_malloc(sizeof(AVPacket));
  
  av_read_frame(fmtCtx, packet);
  memcpy(src_data[0], packet->data, packet->size);
  sws_scale(sws_ctx, src_data, src_linesize, 0, pCodecCtx->height, dst_data, dst_linesize);
 
  
  /**
   *  code below here encode output jpeg
   * 
   */
  AVOutputFormat *pOutFormat;
  AVFormatContext *pFormatCtx;
  AVCodecContext *pAVCodecCtx;
  AVStream *pOStream;
  AVCodec *pAVcodec;
  AVFrame *pFrame;
  
  std::cout<<"in save jpeg packet size "<<packet->size<<std::endl;
  const char *out_file="out.jpg";
  
  
  pOutFormat = av_guess_format("mjpeg",NULL, NULL);
  if (!pOutFormat){
    std::cout<<"av guess format failed, now return. "<<std::endl;
    return -1;
  }
  /*
  pFormatCtx = avformat_alloc_context();
  if (!pFormatCtx){
    std::cout<<"out formatcontext alloc failed, now return. "<<std::endl;
    return -1;
  }
  
  pFormatCtx->oformat = pOutFormat;
  
  if (avio_open(&(pFormatCtx->pb), out_file, AVIO_FLAG_READ_WRITE)<0){
    std::cout<<"avio open failed, now return. "<<std::endl;
    return -1;
  }
  */
  
  avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);  
  pOutFormat = pFormatCtx->oformat;  
  
  pOStream = avformat_new_stream(pFormatCtx, NULL);
  if (!pOStream){
    std::cout<<"new stream failed, now return. "<<std::endl;
    return -1;
  }
  
  pAVCodecCtx = pOStream->codec;
  pAVCodecCtx->codec_id = pOutFormat->video_codec;
  pAVCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
  pAVCodecCtx->pix_fmt = dst_pix_fmt;
  pAVCodecCtx->width = dst_w;
  pAVCodecCtx->height = dst_h;
  pAVCodecCtx->time_base.num =1;
  pAVCodecCtx->time_base.den = 25;
  
  std::cout<<" out ";
  av_dump_format(pFormatCtx, 0, out_file, 1);
  
  std::cout<<"out codec id "<<pAVCodecCtx->codec_id<<std::endl;
  
  pAVcodec = avcodec_find_encoder(pAVCodecCtx->codec_id);
  if (!pAVcodec){
    std::cout<<"find avcodec failed. "<<std::endl;
    return -1;
  }
  
  if(avcodec_open2(pAVCodecCtx, pAVcodec, NULL) < 0){
    std::cout<<"avcodec open failed. "<<std::endl;
    return -1;
  }
  
  int size = avpicture_get_size(pAVCodecCtx->pix_fmt, pAVCodecCtx->width, pAVCodecCtx->height);
  std::cout<<"size "<<size<<std::endl;
  
  pFrame = av_frame_alloc();
  if (!pFrame){
    std::cout<<"out frame alloc failed, now return. "<<std::endl;
    return -1;
  }
  uint8_t *pFrame_buf = (uint8_t *)av_malloc(size);
  
  if (!pFrame_buf){
    std::cout<<"frame buf alloc failed. "<<std::endl;
    return -1;
  }
  
  avpicture_fill((AVPicture *)pFrame, pFrame_buf, pAVCodecCtx->pix_fmt, pAVCodecCtx->width, pAVCodecCtx->height);
  
  memcpy(pFrame_buf, dst_data[0], dst_bufsize);
  std::cout<<"dst bufsize "<<dst_bufsize<<std::endl;
  
  avformat_write_header(pFormatCtx, NULL);
  
  int y_size = pAVCodecCtx->width * pAVCodecCtx->height;
  std::cout<<"y size "<<y_size<<std::endl;
  
 /* 
  * code not work well
  pFrame->data[0] = pFrame_buf;              
  pFrame->data[1] = pFrame_buf+ y_size;       
  pFrame->data[2] = pFrame_buf+ y_size*5/4;    */

  AVPacket pkt;
  int got_picture;
  
  //Encode  
  ret = avcodec_encode_video2(pAVCodecCtx, &pkt,pFrame, &got_picture);  
  
  if(ret < 0){  
      printf("Encode Error.\n");  
      return -1;  
  }  
  if (got_picture==1){  
      pkt.stream_index = pOStream->index;  
      ret = av_write_frame(pFormatCtx, &pkt);  
  }  
  
  av_free_packet(&pkt);  
    //Write Trailer  
  av_write_trailer(pFormatCtx);  

  printf("Encode Successful.\n");  

  if (pOStream){  
      avcodec_close(pOStream->codec);  
      av_free(pFrame);  
      av_free(pFrame_buf);  
  }  
  avio_close(pFormatCtx->pb);  
  avformat_free_context(pFormatCtx);  
  
  av_free_packet(packet);
  av_freep(&dst_data[0]);
  sws_freeContext(sws_ctx);
  avformat_close_input(&fmtCtx);
}

int main(int argc, char **argv) {
  
  av_register_all();
  avformat_network_init();
  avcodec_register_all();
  avdevice_register_all();
  if (captureOneFrame()<0){
    std::cout<<"capture frame failed, now return. "<<std::endl;
    return -1;
  }
    
  return 0;
}

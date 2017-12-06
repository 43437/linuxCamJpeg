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
  
void captureOneFrame(){
  
  AVFormatContext *fmtCtx = NULL;
  AVInputFormat *inputFmt;
  AVPacket *packet;
  AVCodecContext *pCodecCtx;
  AVCodec *pCodec;
  struct SwsContext *sws_ctx;
  
  FILE *fp;
  int i;
  int ret;
  int videoindex;
  
  enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_YUV420P;
  const char *dst_size = NULL;
  const char *src_size = NULL;
  uint8_t *src_data[4];
  uint8_t *dst_data[4];
  int src_linesize[4];
  int dst_linesize[4];
  int src_bufsize;
  int dst_bufsize;
  
  int src_w;
  int src_h;
  int dst_w = 1280;
  int dst_h = 960;
  
  fp = std::fopen(out_file, "wb");
  if (fp < 0){
    std::cout<<"open out file failed, now return"<<std::endl;
    return;
  }
  
  inputFmt = av_find_input_format(input_name);
  if (inputFmt == NULL){
    std::cout<<"can not find input format. "<<std::endl;
    return;
  }
  
  if (avformat_open_input(&fmtCtx, file_name, inputFmt, NULL)<0){
    std::cout<<"can not open input file."<<std::endl;
    return;
  }
  
  av_dump_format(fmtCtx, 0, file_name, 0);
  
  videoindex = -1;
  for (i=0;i<fmtCtx->nb_streams;i++){
    if(fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
      videoindex = i;
      break;
    }
  }
  
  if(videoindex == -1){
    std::cout<<"find video stream failed, now return."<<std::endl;
    return;
  }
  
  pCodecCtx = fmtCtx->streams[videoindex]->codec;
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  
  std::cout<<"picture width height format"<<pCodecCtx->width<<pCodecCtx->height<<pCodecCtx->pix_fmt<<std::endl;
  
  sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt
    ,dst_w, dst_h, dst_pix_fmt, SWS_BILINEAR, NULL, NULL, NULL);
  
  src_bufsize = av_image_alloc(src_data, src_linesize, pCodecCtx->width, pCodecCtx->height
    , pCodecCtx->pix_fmt, 16);
  
  dst_bufsize = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pix_fmt, 1);
  
  packet = (AVPacket *)av_malloc(sizeof(AVPacket));
  
  av_read_frame(fmtCtx, packet);
  memcpy(src_data[0], packet->data, packet->size);
  sws_scale(sws_ctx, src_data, src_linesize, 0, pCodecCtx->height, dst_data, dst_linesize);
  std::fwrite(dst_data[0], 1, dst_bufsize, fp);
  
  std::fclose(fp);
  av_free_packet(packet);
  av_freep(&dst_data[0]);
  sws_freeContext(sws_ctx);
  avformat_close_input(&fmtCtx);
}
int main(int argc, char **argv) {
  
  avcodec_register_all();
  avdevice_register_all();
  captureOneFrame();
    
  return 0;
}

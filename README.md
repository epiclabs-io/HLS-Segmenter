# iPhone HLS Segmenter

The segmenter will take an VIDEO input stream (a ffmpeg url, file or stdin) and packet-copy it to HLS segments. 
It will also generate .m3u8 playlist 

If output is under httpd document root files are ready to stream. 

The input should be MPEG TS. The old code dealing with .mp3 and raw aac streams was removed!


This project is based on old version of http://www.ioncannon.net/projects/http-live-video-stream-segmenter-and-distributor/
which was heavily modified for stability. It is possible to segment "live" streams from stdin or any ffmpegable url to tmpfs under 
httpd with limited segment numbers 

## HOW TO COMPILE

Install latest FFMPEG version(libav is is probably not supportd), cmake 2.6+ and go to the directory where  CMakeLists.txt is located.

 $ cmake .  
 $ make  
 $ make install  


for example usage try segmenter -h


## DESCRIPTION

This project is an attempt to make it easier to set up a live streaming server using Apple's HTTP streaming protocol.


## REQUIREMENTS

FFMpeg is the only external requirement 


## LICENSE

Copyright (c) 2015 Jesus Oliva
Copyright (c) 2014 Stoian Ivanov  
Copyright (c) 2009 Carson McDonald  
Copyright (c) 2009 Chase Douglas  

This program is free software; you can redistribute it and/or  
modify it under the terms of the GNU General Public License version 2  
as published by the Free Software Foundation.  

This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
GNU General Public License for more details.  

You should have received a copy of the GNU General Public License  
along with this program; if not, write to the Free Software  
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  

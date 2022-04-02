# fb_test
Framebuffer library test app


linux console (hdmi monitor) cursor blink disable.

echo 0 > /sys/class/graphics/fbcon/cursor_blink

Linux Server Image Screen saver off 방법(HDMI모니터 연결시 일정시간 후 Blank)

setterm -blank 0 -powersave off

/etc/bash.bashrc에 넣는 경우 login할 때 마다 명령을 실행하게 됨. 
ssh로 접속시에는 아래의 에러가 발생됨

setterm: terminal xterm-256color does not support --blank
setterm: cannot (un)set powersave mode: Inappropriate ioctl for device

에러 발생을 없애기 위하여 아래와 같이 진행함.

setterm -blank 0 -powersave off 2>/dev/null

linemon: linemon.h linemon.c sip.c
	gcc -g -o linemon linemon.c sip.c `pkg-config libpjproject --cflags --libs`

test:
	./linemon "100:pass@172.28.128.3" **1

plot:
	sox -M -r 8000 -e unsigned -b 16 -c 1 tx_out.raw -r 8000 -e unsigned -b 16 -c 1 rx_out.raw out.wav
	sox out.wav -n spectrogram
	display spectrogram.png

clean:
	rm -f linemon *.wav *.raw *.png *~

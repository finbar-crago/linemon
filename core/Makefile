linemon: linemon.c
	gcc -g -o linemon linemon.c `pkg-config libpjproject --cflags --libs`

plot:
	sox -M -r 8000 -e unsigned -b 16 -c 1 tx_out.raw -r 8000 -e unsigned -b 16 -c 1 rx_out.raw out.wav
	sox out.wav -n spectrogram
	display spectrogram.png

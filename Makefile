all: read write

write: applicationLayer.c open.c read.c write.c writenoncanonical.c
			gcc applicationLayer.c open.c write.c read.c writenoncanonical.c -Wall -o write

read: applicationLayer.c open.c read.c write.c noncanonical.c
			gcc applicationLayer.c open.c write.c read.c noncanonical.c -Wall -o read

clean:
	-rm -rf read
	-rm -rf write

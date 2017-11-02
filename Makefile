all: read write

write: applicationLayer.c open.c read.c write.c writenoncanonical.c close.c
			gcc applicationLayer.c open.c write.c read.c writenoncanonical.c close.c -Wall -o write

read: applicationLayer.c open.c read.c write.c noncanonical.c close.c
			gcc applicationLayer.c open.c write.c read.c noncanonical.c close.c -Wall -o read

clean:
	-rm -rf read
	-rm -rf write

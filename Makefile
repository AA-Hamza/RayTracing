CC=g++
CCFLAGS=-pthread -O3
IMAGE=image.ppm
IMAGE_JPEG=image.jpeg

all: raytracer

raytracer: main.cpp
	$(CC) $^ $(CCFLAGS) -o $@

image: raytracer
	./$< > $(IMAGE)

jpeg: 
	convert $(IMAGE) $(IMAGE_JPEG)

clean:
	rm -fr raytracer

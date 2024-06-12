
all: 
	echo "select one of the other targets"

build-x86-image:
	docker buildx build --platform linux/amd64 --tag ghcr.io/tritonuas/obcpp:x86 --file Dockerfile.x86 ..

# NOTE: the jetson Docker image is based on a custom base image we generate from  
# https://github.com/dusty-nv/jetson-containers/
# - Clone that repo and run the following command: 
#     ./build.sh --name=tritonuas/jetson-base pytorch:2.1 torchvision opencv 
# - Push that image to dockerhub as whatever tag the FROM line in our Dockerfile.jetson is
# - Run the following build-jeston-image target
build-jetson-image:
	docker buildx build --platform linux/arm64 --tag ghcr.io/tritonuas/obcpp:jetson --file Dockerfile.jetson ..

build-arm-image:
	docker buildx build --platform linux/arm64 --tag ghcr.io/tritonuas/obcpp:arm --file Dockerfile.arm ..

push-jetson-image:
	docker push ghcr.io/tritonuas/obcpp:jetson

run-jetson-pixhawk-compose:
	docker compose -f jetson-pixhawk-compose.yml up -d

stop-jetson-pixhawk-compose:
	docker compose -f jetson-pixhawk-compose.yml down

run-jetson-cuda-check:
	docker run -it --rm --runtime nvidia -i ghcr.io/tritonuas/obcpp:jetson /obcpp/build/bin/cuda_check

<<<<<<< HEAD

# NOTE: Use this target for development where you want to quicly edit source code and recompile.
# This will spawn up the jetson container, launch you into a bash shell and mount the 
# host's obcpp directory at "/obcpp" in the container. This means you can edit the source 
# files on the host (with VSCode over SSH or Vim) and recompile in the Docker environment.
# Note that to re-run cmake inside the container you'll need the really long CMake command
# in the Dockerfile.jetson.
jetson-develop:
	cd .. && docker run -it --net=host --runtime=nvidia --platform=linux/arm64 --volume=./:/obcpp --device=/dev/ttyACM0 tritonuas/obcpp:jetson /bin/bash
=======
jetson-develop:
	cd .. && docker run -it --net=host --runtime=nvidia --volume=./:/obcpp -i tritonuas/obcpp:jetson /bin/bash
>>>>>>> bdbd1f8 (pull)

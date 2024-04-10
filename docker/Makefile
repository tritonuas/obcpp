
all: 
	echo "select one of the other targets"

# NOTE: the jetson Docker image is based on a custom base image we generate from  
# https://github.com/dusty-nv/jetson-containers/
# - Clone that repo and run the following command: 
#     ./build.sh --name=tritonuas/jetson-base pytorch:2.1 torchvision opencv 
# - Push that image to dockerhub as whatever tag the FROM line in our Dockerfile.jetson is
# - Run the following build-jeston-image target
build-jetson-image:
	docker buildx build --platform linux/arm64 --tag tritonuas/obcpp:jetson --file Dockerfile.jetson ..

push-jetson-image:
	docker push tritonuas/obcpp:jetson

build-arm-image:
	DOCKER_BUILDKIT=1 docker build --tag tritonuas/obcpp:arm --file Dockerfile.arm ..

run-jetson-pixhawk-compose:
	docker compose -f jetson-pixhawk-compose.yml up -d

stop-jetson-pixhawk-compose:
	docker compose -f jetson-pixhawk-compose.yml down

run-jetson-cuda-check:
	docker run -it --rm --runtime nvidia -i tritonuas/obcpp:nvidia /obcpp/build/bin/cuda_check


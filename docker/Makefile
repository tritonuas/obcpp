
all: 
	echo "select one of the other targets"

build-jetson-image:
	docker buildx build --platform linux/arm64 --tag tritonuas/obcpp:jetson --file Dockerfile.jetson ..

build-arm-image:
	DOCKER_BUILDKIT=1 docker build --tag tritonuas/obcpp:arm --file Dockerfile.arm ..

run-jetson-pixhawk-compose:
	docker compose -f jetson-pixhawk-compose.yml up -d

stop-jetson-pixhawk-compose:
	docker compose -f jetson-pixhawk-compose.yml down

run-jetson-cuda-check:
	docker run -it --rm --runtime nvidia -i tritonuas/obcpp:nvidia /obcpp/build/bin/cuda_check



all: 
	echo "select one of the other targets"

build-jetson-image:
	DOCKER_BUILDKIT=1 docker build --tag tritonuas/obcpp:nvidia --file Dockerfile.nvidia ..

build-arm-image:
	DOCKER_BUILDKIT=1 docker build --tag tritonuas/obcpp:arm --file Dockerfile.arm ..

run-jetson-cuda-check:
	docker run -it --rm --runtime nvidia -i tritonuas/obcpp:nvidia /obcpp/build/bin/cuda_check

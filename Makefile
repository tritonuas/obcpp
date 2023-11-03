# =============================
# Docker Build
.PHONY: build-docker-x86
build-docker-x86:
	docker build -t tritonuas/obcpp:x86 -f ./docker/Dockerfile.x86 .

.PHONY: build-docker-arm
build-docker-arm:
	docker build --tag tritonuas/obcpp:arm -f ./docker/Dockerfile.arm .

.PHONY: build-docker-nvidia
build-docker-nvidia:
	docker build --tag tritonuas/obcpp:nvidia -f ./docker/Dockerfile.nvidia .
# =============================

# =============================
# Docker Run
.PHONY: run-docker-x86
run-docker-x86:
	docker run --rm -it tritonuas/obcpp:x86

.PHONY: run-docker-arm
run-docker-arm:
	docker run --rm -it tritonuas/obcpp:arm

.PHONY: run-docker-nvidia
run-docker-nvidia:
	docker run --rm -it --network=host --runtime=nvidia tritonuas/obcpp:nvidia
# =============================

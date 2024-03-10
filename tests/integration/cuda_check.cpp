#include <torch/torch.h>

int main() {
	std::cout << "Have CUDA? " << torch::cuda::is_available() << std::endl;
}

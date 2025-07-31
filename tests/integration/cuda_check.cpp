#include <torch/torch.h>

std::string bool_to_string(bool b) {
	return b ? "true" : "false";
}

int main() {
	std::cout << "Have CUDA? " << bool_to_string(torch::cuda::is_available()) << std::endl;
}

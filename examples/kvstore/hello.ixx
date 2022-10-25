export module hello;

import <iostream>;

export void greeter(const char *name) {
  std::cout << "Hello " << name << std::endl;
}
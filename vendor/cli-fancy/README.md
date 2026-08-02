# cli-fancy library

Single header library for cli apps

Used in ctrff and npi-build

## Example

```cpp
#include <cli-fancy.hpp>

void ShowString(const cf7::command::ArgumentList& data) {
  std::cout << "Input String was: " << cf7::command::GetArg(data, "input") << std::endl;
}

int main(int argc, char* argv[]) {
  cf7::arg_mgr mgr(argc, argv);
  mgr.SetAppInfo("test", "1.0.0");
  mgr.AddCommand(
      cf7::command("show", "Show an input String")
          .AddSubEntry(cf7::command::sub("i", "input", "Input String", true))
          .SetFunction(ShowString));
  mgr.Execute();
  return 0;
}
```

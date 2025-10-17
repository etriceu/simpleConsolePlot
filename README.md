# simpleConsolePlot
Simple Header-only C++ Console/Terminal Plot Library

The documentation is available in the .hpp file.

# Requirements
- Terminal with unicode support
- Linux (probably other Unixes as well)

# Examples
<img width="539" height="453" alt="image" src="https://github.com/user-attachments/assets/e8b68bea-f837-4abf-aa76-67605c723ebf" />

```cpp
#include "simpleConsolePlot.hpp"
using namespace SCP;

int main() {
	Plot p(70, 30); // 70 characters wide, 30 line high
	
	p.setBackgroundColor(DARK_GRAY);
	p.setXAxisFormat();
	p.setYAxisFormat("%5.2f");
	p.setDrawRange(-3, -1, 3, 1);
	
	for(double x = -3; x < 3; x += 0.5) {
		p.line(x, -tanh(x), x+0.5, -tanh(x+0.5), GREEN);
		p.point(x, -tanh(x));
		//p.point(x, -tanh(x), RED, 'X');
	}
	p.line(-3, 0, 3, 0, BLACK, '-');
	p.line(0, -1, 0, 1, BLACK, '|');
	
	p.render();
	p.print();
	//p.clearData();
	
	return 0;
} 
```
[sine.webm](https://github.com/user-attachments/assets/bb900a17-e1ba-4c55-bfce-172b1d9b70fc)
```cpp
#include "simpleConsolePlot.hpp"
#include <unistd.h>

using namespace SCP;

int main() {
	Plot p;
	p.setSize(125, 40);
	p.setDrawRange(0, -1, 2*3.1415, 1);
	
	double xx = 0;
	for(double xx = 0;; xx += 0.025) {
		printf("\033[2J\033[H"); //clear screen
		p.clearData();
		
		for(double x = 0; x < 2*3.1415; x += 0.25)
			p.line(x, -sin(x+xx), x+0.25, -sin(x+xx+0.25), GREEN);

		p.print();
		usleep(20000);
	}
	
	return 0;
}
```

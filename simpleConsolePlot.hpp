#ifndef SIMPLE_CONSOLE_PLOT_HPP
#define SIMPLE_CONSOLE_PLOT_HPP

#include <cstdio>
#include <cmath>
#include <vector>
#include <limits>
#include <unordered_map>
#include <string>

namespace SCP {

enum {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, BRIGHT_GRAY, DARK_GRAY,
	BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW, BRIGHT_BLUE, BRIGHT_MAGENTA,
	BRIGHT_CYAN, WHITE};

class Plot {
public:
	enum{EMPTY=' ', BLOCK='\0'};
	struct Cell {
		char ch;
		int8_t co;
		
		bool operator==(const Cell &a) const {
			return a.ch == ch && a.co == co;
		}
	};
	
	struct CellHash {
		std::size_t operator()(const Cell &a) const {
			return static_cast<std::size_t>(a.ch) | 
			(static_cast<std::size_t>(a.co)<<8);
		}
	};
	
	struct Point {
		double x, y;
	};
	
	struct Line {
		Point a, b;
	};
	
	int w = 10, h = 10;
	Cell *printBuf = nullptr;
	int8_t background = BLACK;
	bool range = false;
	bool invertedY = false;
	Point topLeft;
	double dx, dy;
	double minX = std::numeric_limits<typeof(minX)>::infinity(),
		maxX = -minX, minY = minX, maxY = -minX;
	
	std::unordered_map<Cell, std::vector<Point>, CellHash> points;
	std::unordered_map<Cell, std::vector<Line>, CellHash> lines;
	
	std::string yFormat = "", xFormat = "";
	
public:
	Plot() = default;
	
	/**
	 * Creates a plot of the specified size in characters and lines.
	 * When displaying default blocks, the line is treated as two rows.
	 * 
	 * @param length Width/length of the plot in the number of characters.
	 * @param lines Height of the plot in the number of lines.
	 */
	Plot(int length, int lines) {
		setSize(length, lines);
	}
	
	~Plot() {
		if(printBuf != nullptr)
			delete [] printBuf;
	}
	
	/**
	 * Inverts the Y axis.
	 * By default, Y increases downwards.
	 * @param a When true, the axis will be inverted.
	 */
	void invertYAxis(bool a = false) {
		invertedY = a;
	}
	
	/**
	 * Sets the display format of the values on the X axis according to the printf options.
	 * If empty, the axis will not be displayed.
	 * 
	 * @param format Number format, compliant with printf.
	 */
	void setXAxisFormat(std::string format = "%6.2f") {
		xFormat = format;
	}
	
	/**
	 * Sets the display format of the values on the Y axis according to the printf options.
	 * If empty, the axis will not be displayed.
	 * 
	 * @param format Number format, compliant with printf.
	 */
	void setYAxisFormat(std::string format = "%6.2f") {
		yFormat = format;
	}
	
	/**
	 * Sets the size of the plot in characters and lines.
	 * When displaying default blocks, the line is treated as two rows.
	 * 
	 * @param length Width/length of the plot in the number of characters.
	 * @param lines Height of the plot in the number of lines.
	 */
	void setSize(int length, int lines) {
		w = length;
		h = lines;
		
		if(printBuf != nullptr)
			delete [] printBuf;
		
		printBuf = new Cell[w*h];
		clearPlot();
	}
	
	/**
	 * Sets the range of data to be displayed on the plot area.
	 * Calling without arguments will cause the entire plot to be displayed.
	 * 
	 * @param x1 The minimum X-coordinate to display on the plot.
	 * @param y1 The minimum Y-coordinate to display on the plot.
	 * @param x2 The maximum X-coordinate to display on the plot.
	 * @param y2 The maximum Y-coordinate to display on the plot.
	 */
	void setDrawRange(double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0) {
		topLeft = {x1, y1};
		dx = x2-x1;
		dy = y2-y1;
		range = dx != 0;
		clearPlot();
	}
	
	/**
	 * Sets the background color of the plot.
	 * 
	 * @param color The color to set as the background.
	 */
	void setBackgroundColor(int8_t color) {
		background = color;
	}
	
	/**
	 * Removes all plot data.
	 */
	void clearData() {
		clearPlot();
		
		minX = std::numeric_limits<typeof(minX)>::infinity();
		minY = minX;
		maxX = -minX;
		maxY = -minX;
		
		points.clear();
		lines.clear();
	}
	
	/**
	 * Adds a point to the plot.
	 * When the drawing range is set, the point can also be drawn in the buffer.
	 * @param x The X-coordinate of the point.
	 * @param y The Y-coordinate of the point.
	 * @param color The color of the point, defaulting to WHITE.
	 * @param character The character to be used for drawing the point, by default, is the Unicode square/block character.
	 */
	void point(double x, double y, int8_t color = WHITE, char character = '\0') {
		Point p = {x, y};
		points[{character, color}].push_back(p);
		
		if(range)
			printPoint(p, color, character);
		else
			updateXYMinMax(p);
	}
	
	/**
	 * Adds a line to the plot.
	 * When the drawing range is set, the line can also be drawn in the buffer.
	 * @param x1 The X-coordinate of the starting point of the line.
	 * @param y1 The Y-coordinate of the starting point of the line.
	 * @param x2 The X-coordinate of the ending point of the line.
	 * @param y2 The Y-coordinate of the ending point of the line.
	 * @param color The color of the line, defaulting to WHITE.
	 * @param character The character to be used for drawing the line, by default, is the Unicode square/block character.
	 */
	void line(double x1, double y1, double x2, double y2, int8_t color = WHITE, 
			  char character = '\0') {
		Line l = {{x1, y1}, {x2, y2}};
		lines[{character, color}].push_back(l);
		
		if(range)
			printLine(l, color, character);
		else {
			updateXYMinMax(l.a);
			updateXYMinMax(l.b);
		}
	}
	
	/**
	 * Renders the plot to the buffer.
	 */
	void render() {
		if(!range) {
			dx = maxX-minX;
			dy = maxY-minY;
			topLeft = {minX, minY};
		}
		
		for(const auto &c : lines)
			for(const auto &l : c.second)
				printLine(l, c.first.co, c.first.ch);
		
		for(const auto &c : points)
			for(const auto &p : c.second)
				printPoint(p, c.first.co, c.first.ch);
	}
	
	/**
	 * Prints the buffered plot to stdout.
	 */
	void print() {
		int8_t last;
		int startY = 0, endY = h, step = 1;
		
		if(invertedY) {
			startY = h-1;
			endY = -1;
			step = -1;
		}
		
		for(int y = startY; y != endY; y += step) {
			Cell *c = printBuf+y*w;
			for(int x = 0; x < w; x++, c++) {
				if(last != c->co || x == 0) {
					printf("\x1b[%d%d;%d%dm", c->co&0x08?9:3, c->co&0x07, 
						   c->co&0x80?10:4, c->co>>4&0x07);
					last = c->co;
				}
				
				if(c->ch == BLOCK)
					printf("\u2580");
				else
					putchar(c->ch);
			}
			if(!yFormat.empty()) {
				printf("\x1b[0m");
				printf(yFormat.c_str(), topLeft.y+y*dy/h);
			}
			putchar('\n');
		}
		printf("\x1b[0m");
		if(!xFormat.empty()) {
			for(int x = 0; x < w;) {
				putchar('|');
				x += printf(xFormat.c_str(), topLeft.x+x*dx/w)+1;
			}
			putchar('\n');
		}
	}

private:
	void clearPlot() {
		if(printBuf == nullptr)
			return;
		
		for(Cell *it = printBuf, *end = it+w*h; it < end; it++)
			*it = {EMPTY, static_cast<int8_t>(background<<4|background)};
	}
	
	void updateXYMinMax(const Point &p) {
		if(minX > p.x) minX = p.x;
		if(minY > p.y) minY = p.y;
		if(maxX < p.x) maxX = p.x;
		if(maxY < p.y) maxY = p.y;
	}
	
	void setCell(int x, int y, int8_t color, char character) {
		if(x < 0 || x >= w || y < 0 || y >= h*2)
			return;
		
		Cell &c = printBuf[x+(y/2)*w];
		if(character == '\0') {
			if(c.ch == EMPTY)
				c.ch = BLOCK;
			
			if(invertedY)
				y++;
			c.co = y%2 ? c.co&0x0f|color<<4 : c.co&0xf0|color;
		}
		else {
			if(c.ch == EMPTY)
				c.co = c.co&0xf0|color;
			else if(c.ch == BLOCK)
				c.co = (c.co&0x0f) != background ? c.co<<4|color : c.co&0xf0|color;
			else if(c.ch != EMPTY && c.ch != character)
				c.co = c.co<<4|color;
				
			c.ch = character;
		}
	}
	
	void printPoint(const Point &p, int8_t color, char character) {
		setCell((p.x-topLeft.x)*w/dx, (p.y-topLeft.y)*h*2/dy, color, character);
	}
	
	void printLine(const Line &l, int8_t color, char character) {
		int x1 = (l.a.x-topLeft.x)*w/dx;
		int y1 = (l.a.y-topLeft.y)*h*2/dy;
		const int x2 = (l.b.x-topLeft.x)*w/dx;
		const int y2 = (l.b.y-topLeft.y)*h*2/dy;
		const int dx = abs(x2-x1);
		const int dy = abs(y2-y1);
		const int sx = (x1 < x2) ? 1 : -1;
		const int sy = (y1 < y2) ? 1 : -1;
		int e1 = dx-dy;
		
		while(true) {
			setCell(x1, y1, color, character);
			if(x1 == x2 && y1 == y2)
				break;
			
			int e2 = e1*2;
			if(e1 > -dy) {
				e1 -= dy;
				x1 += sx;
			}
			if(e2 < dx) {
				e1 += dx;
				y1 += sy;
			}
		}
	}
};

}

#endif // SIMPLE_CONSOLE_PLOT_HPP

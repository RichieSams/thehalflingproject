/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef CONSOLE_PROGRESS_BAR_H
#define CONSOLE_PROGRESS_BAR_H

#include "common/typedefs.h"

#include "rlutil.h"
#include <cassert>


namespace Common {

class ConsoleProgressBar {
public:
	ConsoleProgressBar();

private:
	uint m_numCols;
	uint m_verticalOffset;

public:
	void DrawProgress(uint progress);
	void SetVerticalOffset(uint offset);
};

ConsoleProgressBar::ConsoleProgressBar() 
	: m_numCols(rlutil::tcols()),
	  m_verticalOffset(1) {
}

void ConsoleProgressBar::DrawProgress(uint progress) {
	rlutil::locate(1, 3);
	std::cout << progress << " %" << std::endl << std::endl;

	rlutil::locate(7, m_verticalOffset);
	uint numberOfTicks = progress * (m_numCols - 6) / 100;
	for (uint i = 0; i < numberOfTicks; ++i) {
		std::cout << (char)(219);
	}
	for (uint i = numberOfTicks; i < m_numCols - 6; ++i) {
		std::cout << ' ';
	}
}

void ConsoleProgressBar::SetVerticalOffset(uint offset) {
	assert(offset <= rlutil::trows());
	m_verticalOffset = offset;
}

}

#endif
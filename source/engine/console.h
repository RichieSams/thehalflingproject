/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "graphics/sprite_renderer.h"
#include "graphics/sprite_font.h"
#include "common/rect.h"

#include <deque>
#include <unordered_map>
#include <d3d11.h>


namespace Engine {

typedef void (*ConsoleCommandCallback)(std::deque<std::wstring> args);

class Console {
public:
	Console();

private:
	Common::Rect m_rect;
	Graphics::SpriteRenderer *m_spriteRenderer;
	Graphics::SpriteFont *m_font;
	uint m_maxNumLines;

	std::deque<std::wstring> m_lines;

	std::wstring m_currentInput;
	bool m_cursorShowing;
	
	double m_accumulatedCursorTime;
	static const double kCursorBlinkFrequency;

	std::unordered_map<std::wstring, ConsoleCommandCallback> m_commandCallbacks;

public:
	void Initialize(Common::Rect rect, Graphics::SpriteRenderer *spriteRenderer, Graphics::SpriteFont *font, uint maxNumLines = 50U);
	void Render(ID3D11DeviceContext *context, double deltaTime);
	void PrintText(const std::wstring &line);
	void InputCharacter(wchar character);

	bool RegisterCommand(const std::wstring &command, ConsoleCommandCallback callback);

private:
	void ProcessCommandString(std::wstring commandString);
};

} // End of namespace Engine

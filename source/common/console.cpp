/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/console.h"

#include "common/string_util.h"

#include <DirectXMath.h>


namespace Common {

const double Console::kCursorBlinkFrequency = 1000.0;

Console::Console()
	: m_currentInput(L"> "),
	  m_accumulatedCursorTime(0.0) {
}

void Console::Initialize(Common::Rect rect, Common::SpriteRenderer *spriteRenderer, Common::SpriteFont *font, uint maxNumLines) {
	m_rect = rect;
	m_spriteRenderer = spriteRenderer;
	m_font = font;
	m_maxNumLines = maxNumLines;
}

void Console::Render(ID3D11DeviceContext *context, double deltaTime) {
	// Figure out if the cursor is shown
	m_accumulatedCursorTime += deltaTime;

	while (m_accumulatedCursorTime >= kCursorBlinkFrequency) {
		m_accumulatedCursorTime -= kCursorBlinkFrequency;
		m_cursorShowing = !m_cursorShowing;
	}

	m_spriteRenderer->Begin(context, SpriteRenderer::Point);
	
	// Draw the background rectangle
	DirectX::XMFLOAT4X4 backgroundTransform;
	DirectX::XMStoreFloat4x4(&backgroundTransform, DirectX::XMMatrixScaling(m_rect.GetWidth() + 20.0f, static_cast<float>(m_rect.GetHeight()), 0.0f));
	backgroundTransform._41 = (float)m_rect.Left - 10.0f;
	backgroundTransform._42 = (float)m_rect.Top;

	m_spriteRenderer->Render(nullptr, backgroundTransform, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.75f));

	DirectX::XMFLOAT4X4 transform {1, 0, 0, 0,
	                               0, 1, 0, 0,
	                               0, 0, 1, 0,
	                               (float)m_rect.Left, (float)m_rect.Bottom, 0, 1};
	
	// Draw the input line
	DirectX::XMFLOAT2 inputSize = m_font->MeasureText(m_currentInput.c_str(), m_rect.GetWidth());
	transform._42 -= inputSize.y;
	DirectX::XMFLOAT2 cursorLocation = m_spriteRenderer->RenderText(*m_font, m_currentInput.c_str(), transform, m_rect.GetWidth(), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

	// Render the cursor
	if (m_cursorShowing) {
		DirectX::XMFLOAT4X4 cursorTransform {1.0f, 0.0f, 0.0f, 0.0f,
		                                     0.0f, 1.0f, 0.0f, 0.0f,
		                                     0.0f, 0.0f, 1.0f, 0.0f,
											 cursorLocation.x, cursorLocation.y, 0.0f, 1.0f};

		m_spriteRenderer->RenderText(*m_font, L"_", cursorTransform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);
	}

	// Draw the lines of text, starting from the newest
	auto iter = m_lines.rbegin();
	while (transform._42 > m_rect.Top && iter != m_lines.rend()) {
		transform._42 -= m_font->CharHeight();
		// We don't have to worry about wrapping because PrintText takes care of that for us
		m_spriteRenderer->RenderText(*m_font, iter->c_str(), transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		++iter;
	}

	m_spriteRenderer->End();
}

void Console::PrintText(const std::wstring &line) {
	uint lastSubstring = 0;
	float currentWidth = 0;

	for (uint i = 0; i < line.size(); ++i) {
		wchar character = line[i];

		switch (character) {
		case '\r':
		case '\n':
		{
			uint count = i - lastSubstring;
			if (count > 0) {
				m_lines.push_back(line.substr(lastSubstring, count));
			}
			// Add 1 so we remove the newline character
			lastSubstring = i + 1;
			currentWidth = 0;
			break;
		}
		case ' ':
			currentWidth += m_font->SpaceWidth();

			if (currentWidth > m_rect.GetWidth()) {
				m_lines.push_back(line.substr(lastSubstring, i - lastSubstring));
				lastSubstring = i;
				currentWidth = 0;
			}
			break;
		case '\t':
			currentWidth += 4.0f * m_font->SpaceWidth();

			if (currentWidth > m_rect.GetWidth()) {
				m_lines.push_back(line.substr(lastSubstring, i - lastSubstring));
				lastSubstring = i;
				currentWidth = 0;
			}
			break;
		default:
		{
			Common::SpriteFont::CharDesc desc = m_font->GetCharDescriptor(character);
			currentWidth += desc.Width;

			if (currentWidth > m_rect.GetWidth()) {
				m_lines.push_back(line.substr(lastSubstring, i - lastSubstring));
				lastSubstring = i;
				currentWidth = 0;
			}
			break;
		}
		} // End of switch
	}

	// Put the last characters into a line
	m_lines.push_back(line.substr(lastSubstring));

	// Pop off lines until we're under the limit
	uint numLinesToPop = static_cast<uint>(m_lines.size()) - m_maxNumLines;
	for (uint i = 0; i < numLinesToPop; ++i) {
		m_lines.pop_front();
	}
}

void Console::InputCharacter(wchar character) {
	switch (character) {
	case '\n':
	case '\r':
	{
		std::wstring command = m_currentInput.substr(2);
		if (!command.empty()) {
			PrintText(m_currentInput);
			ProcessCommandString(command);

			// Reset the current input to 'empty'
			m_currentInput = L"> ";
		}
		break;
	}
	case '\b':
		if (m_currentInput.size() > 2) {
			m_currentInput.pop_back();
		}
		break;
	case '\t':
		break;
	default:
		m_currentInput.push_back(character);
		break;
	}
}

bool Console::RegisterCommand(const std::wstring &command, ConsoleCommandCallback callback) {
	if (command.find(' ') != std::string::npos) {
		std::wstringstream sstream;
		sstream << L"\tRegisterCommand - '" << command << L"': Command name can not contain spaces. ";
		PrintText(sstream.str());
		return false;
	}

	auto iter = m_commandCallbacks.find(command);

	return true;
}

void Console::ProcessCommandString(std::wstring commandString) {
	std::deque<std::wstring> args;
	Common::Tokenize(commandString, args, L" ", true);

	// The user entered only spaces
	// Just reset
	if (args.size() == 0) {
		return;
	}

	// Now try to process it
	std::wstring command = args.front();
	args.pop_front();

	auto iter = m_commandCallbacks.find(command);
	if (iter != m_commandCallbacks.end()) {
		iter->second(args);
	} else {
		std::wstringstream sstream;
		sstream << L"\tCommand '" << command << L"' is not registered";
		PrintText(sstream.str());
	}
}

} // End of namespace Common

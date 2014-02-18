/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_RECT_H
#define COMMON_RECT_H

#include "common/typedefs.h"

#include <cassert>
#include <algorithm>


namespace Common {

/**
 * Simple class for handling a rectangular zone.
*/
struct Rect {
	int Top, Left;
	int Bottom, Right;

	Rect() : Top(0), Left(0), Bottom(0), Right(0) {}
	Rect(int w, int h) : Top(0), Left(0), Bottom(h), Right(w) {}
	Rect(int x1, int y1, int x2, int y2) : Top(y1), Left(x1), Bottom(y2), Right(x2) {
		assert(IsValidRect());
	}
	bool operator==(const Rect &rhs) const { return Equals(rhs); }
	bool operator!=(const Rect &rhs) const { return !Equals(rhs); }

	int GetWidth() const { return Right - Left; }
	int GetHeight() const { return Bottom - Top; }

	void SetWidth(int width) {
		Right = Left + width;
	}

	void SetHeight(int height) {
		Bottom = Top + height;
	}

	/**
	 * Check if given position is inside this rectangle.
	 *
	 * @param x    The horizontal position to check
	 * @param y    The vertical position to check
	 * @return     True if the given position is inside this rectangle, false otherwise
	 */
	bool Contains(int x, int y) const {
		return (Left <= x) && (x < Right) && (Top <= y) && (y < Bottom);
	}

	/**
	 * Check if the given rect is contained inside this rectangle.
	 *
	 * @param r    The rectangle to check
	 * @return     True if the given rect is inside, false otherwise
	 */
	bool Contains(const Rect &other) const {
		return (Left <= other.Left) && (other.Right <= Right) && (Top <= other.Top) && (other.Bottom <= Bottom);
	}

	/**
	 * Check if the given rect is equal to this one.
	 *
	 * @param r    The rectangle to check
	 * @return     True if the given rect is equal, false otherwise
	 */
	bool Equals(const Rect &other) const {
		return (Left == other.Left) && (Right == other.Right) && (Top == other.Top) && (Bottom == other.Bottom);
	}

	/**
	 * Check if given rectangle intersects with this rectangle
	 *
	 * @param r    The rectangle to check
	 * @return     True if the given rectangle is inside the rectangle, false otherwise
	 */
	bool Intersects(const Rect &other) const {
		return (Left < other.Right) && (other.Left < Right) && (Top < other.Bottom) && (other.Top < Bottom);
	}

	/**
	 * Find the intersecting rectangle between this rectangle and the given rectangle
	 *
	 * @param r    The intersecting rectangle
	 * @return     The intersection of the rectangles or an empty rectangle if not intersecting
	 */
	Rect FindIntersectingRect(const Rect &other) const {
		if (!Intersects(other))
			return Rect();

		return Rect(std::max(other.Left, Left), std::max(other.Top, Top), std::min(other.Right, Right), std::min(other.Bottom, Bottom));
	}

	/**
	 * Extend this rectangle so that it contains r
	 *
	 * @param r    The rectangle to extend by
	 */
	void Extend(const Rect &other) {
		Left = std::min(Left, other.Left);
		Right = std::max(Right, other.Right);
		Top = std::min(Top, other.Top);
		Bottom = std::max(Bottom, other.Bottom);
	}

	/**
	 * Extend this rectangle in all four directions by the given number of pixels
	 *
	 * @param offset    The size to grow by
	 */
	void Grow(int offset) {
		Top -= offset;
		Left -= offset;
		Bottom += offset;
		Right += offset;
	}

	void Clip(const Rect &r) {
		assert(IsValidRect());
		assert(r.IsValidRect());

		if (Top < r.Top) {
			Top = r.Top;
		} else if (Top > r.Bottom) {
			Top = r.Bottom;
		}

		if (Left < r.Left) {
			Left = r.Left;
		} else if (Left > r.Right) {
			Left = r.Right;
		}

		if (Bottom > r.Bottom) {
			Bottom = r.Bottom;
		} else if (Bottom < r.Top) {
			Bottom = r.Top;
		}

		if (Right > r.Right) {
			Right = r.Right;
		} else if (Right < r.Left) {
			Right = r.Left;
		}
	}

	void Clip(int maxWidth, int maxHeight) {
		Clip(Rect(0, 0, maxWidth, maxHeight));
	}

	bool IsEmpty() const {
		return (Left >= Right || Top >= Bottom);
	}

	bool IsValidRect() const {
		return (Left <= Right && Top <= Bottom);
	}

	void MoveTo(int x, int y) {
		Bottom += y - Top;
		Right += x - Left;
		Top = y;
		Left = x;
	}

	void Translate(int dx, int dy) {
		Left += dx; Right += dx;
		Top += dy; Bottom += dy;
	}

	/**
	 * Create a rectangle around the given center
	 * 
	 * @note The center point is rounded up and left when given an odd width and height
	 * 
	 * @param centerX    The x coord of the center point
	 * @param centerY    The y coord of the center point
	 * @param width      The width of the rect
	 * @param height     The height of the rect
	 * @return           The newly created rect
	 */
	static Rect Center(int centerX, int centerY, int width, int height) {
		int x = centerX - width / 2, y = centerY - height / 2;
		return Rect(x, y, x + width, y + height);
	}
};

} // End of namespace Common

#endif

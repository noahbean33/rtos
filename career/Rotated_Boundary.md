# Rotated Boundary Challenge — Firmware C++ Interview Question

## Problem Statement
Given a collection of 2D axis-aligned rectangles:
1. **Part A:** Calculate the global minimum bounding box containing every rectangle
2. **Part B:** Rotate each rectangle 90° around its center; count how many protrude outside the bounding box

## Key Ideas

### Part A — Global Minimum Bounding Box
- Initialize `min_x`, `min_y` to `numeric_limits<float>::max()` and `max_x`, `max_y` to their negatives
- Loop through all rectangles, updating global min/max from each rectangle's corner points
- The bounding box is defined by `(min_x, min_y)` → `(max_x, max_y)`
- Return an empty (0,0,0,0) rectangle if no input rectangles

### Part B — Rotate & Count Violations
- **Center** of each rectangle: `cx = (x1 + x2) / 2.0`, `cy = (y1 + y2) / 2.0`
- **Rotate 90°** = swap width and height: `new_half_width = (y2 - y1) / 2`, `new_half_height = (x2 - x1) / 2`
- Check if rotated rectangle exceeds bounding box on any side:
  - `cx - new_half_width < bbox.x1`
  - `cx + new_half_width > bbox.x2`
  - `cy - new_half_height < bbox.y1`
  - `cy + new_half_height > bbox.y2`
- Increment violation count for any rectangle that fails any check

### Interview Tips
- This is a **dynamic programming / geometry** question, not pure embedded C
- Top firmware positions test **DSA and C++ fluency** alongside low-level skills
- Don't overthink — the straightforward loop-and-compare approach is the intended solution

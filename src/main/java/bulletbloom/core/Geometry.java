package bulletbloom.core;

import java.awt.Rectangle;
import java.awt.geom.Line2D;

/**
 * Geometry helpers for collision checks.
 */
public final class Geometry {
    private Geometry() {
        throw new IllegalStateException("Utility class");
    }

    /**
     * Checks whether a line segment crosses a rectangle.
     *
     * @param x1 segment start x
     * @param y1 segment start y
     * @param x2 segment end x
     * @param y2 segment end y
     * @param rectangle target rectangle
     * @return {@code true} if either endpoint is inside or the segment crosses an edge
     */
    public static boolean segmentIntersectsRectangle(
            double x1,
            double y1,
            double x2,
            double y2,
            Rectangle rectangle) {
        if (rectangle.contains(x1, y1) || rectangle.contains(x2, y2)) {
            return true;
        }
        return Line2D.linesIntersect(x1, y1, x2, y2, rectangle.getMinX(), rectangle.getMinY(), rectangle.getMaxX(), rectangle.getMinY())
                || Line2D.linesIntersect(x1, y1, x2, y2, rectangle.getMaxX(), rectangle.getMinY(), rectangle.getMaxX(), rectangle.getMaxY())
                || Line2D.linesIntersect(x1, y1, x2, y2, rectangle.getMaxX(), rectangle.getMaxY(), rectangle.getMinX(), rectangle.getMaxY())
                || Line2D.linesIntersect(x1, y1, x2, y2, rectangle.getMinX(), rectangle.getMaxY(), rectangle.getMinX(), rectangle.getMinY());
    }
}

package javax.microedition.lcdui.game;

import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

public class TiledLayer extends Layer {
    private int rows;
    private int columns;
    private int tileWidth;
    private int tileHeight;
    private int[] cellMatrix;
    private Image sourceImage;
    private int numberOfTiles;
    private int[] animatedTileTable; // Map animated tile index -> static tile index

    public TiledLayer(int columns, int rows, Image image, int tileWidth, int tileHeight) {
        super(0, 0, columns * tileWidth, rows * tileHeight, true);
        this.columns = columns;
        this.rows = rows;
        this.tileWidth = tileWidth;
        this.tileHeight = tileHeight;
        this.sourceImage = image;
        this.cellMatrix = new int[rows * columns];
        
        int imgW = image.getWidth();
        int imgH = image.getHeight();
        if (imgW % tileWidth != 0 || imgH % tileHeight != 0) {
            throw new IllegalArgumentException("Image size mismatch");
        }
        this.numberOfTiles = (imgW / tileWidth) * (imgH / tileHeight);
        // cellMatrix init to 0 (transparent)
        this.animatedTileTable = new int[0]; // Simplified
    }

    public int createAnimatedTile(int staticTileIndex) {
        // Simplified
        return 0;
    }

    public void setAnimatedTile(int animatedTileIndex, int staticTileIndex) {
        // Simplified
    }

    public int getAnimatedTile(int animatedTileIndex) {
        return 0;
    }

    public void setCell(int col, int row, int tileIndex) {
        if (col < 0 || col >= columns || row < 0 || row >= rows) throw new IndexOutOfBoundsException();
        cellMatrix[row * columns + col] = tileIndex;
    }

    public int getCell(int col, int row) {
        if (col < 0 || col >= columns || row < 0 || row >= rows) throw new IndexOutOfBoundsException();
        return cellMatrix[row * columns + col];
    }

    public void fillCells(int col, int row, int numCols, int numRows, int tileIndex) {
        // Simplified
        for (int r = row; r < row + numRows; r++) {
            for (int c = col; c < col + numCols; c++) {
                setCell(c, r, tileIndex);
            }
        }
    }

    public int getCellWidth() {
        return tileWidth;
    }

    public int getCellHeight() {
        return tileHeight;
    }

    public int getColumns() {
        return columns;
    }

    public int getRows() {
        return rows;
    }

    public final void paint(Graphics g) {
        if (!isVisible()) return;
        
        int x = getX();
        int y = getY();
        
        // Naive painting
        int imgCols = sourceImage.getWidth() / tileWidth;
        
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < columns; c++) {
                int tileIndex = cellMatrix[r * columns + c];
                if (tileIndex == 0) continue;
                
                // Adjust tileIndex for animated tiles (omitted)
                int staticIndex = tileIndex > 0 ? tileIndex - 1 : 0; // 1-based index in TiledLayer, 0 is transparent
                
                int srcCol = staticIndex % imgCols;
                int srcRow = staticIndex / imgCols;
                
                g.drawRegion(sourceImage, srcCol * tileWidth, srcRow * tileHeight, tileWidth, tileHeight, 
                             Sprite.TRANS_NONE, x + c * tileWidth, y + r * tileHeight, Graphics.TOP | Graphics.LEFT);
            }
        }
    }
}

#include "displayheader.h"

/*
    * display.cpp Version 2.0 (with Claude, ChatGPT, GitHub Copilot Help)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * This file implements the DisplayModules class which handles the initialization and management of an SSD1306 display.
    * It includes methods for initializing the display, clearing the screen, displaying strings, and managing a display buffer.
    * The display is used to show information and messages in the ESP32 Attack Tool.
*/

LinkedList<String>* display_buffer;

DisplayModules::DisplayModules() :
    #if defined(_SPI_SCREEN)
        u8g2(U8G2_R0, /* reset=*/ RST_PIN, CS_PIN, DC_PIN),
    #elif defined(_I2C_SCREEN)
        u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, SCL_PIN, SDA_PIN),
    #endif
    screenInitialized(false)
{
}

bool DisplayModules::main()
{  
    if (screenInitialized) {
        Serial.println("[INFO] Display already initialized, skipping...");
        return true;
    }
    display_buffer = new LinkedList<String>();

    Serial.println("[INFO] Initializing Display...");

    #if defined(_SPI_SCREEN)
        // Initialize SPI display
        // Initialize U8g2 with SPI for SSD1306 display
        digitalWrite(CS_PIN, LOW);
        SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_PIN);
        u8g2.setBusClock(16000000); // Set SPI clock speed
    #endif
    // Initialize U8g2 display
    u8g2.begin();
    u8g2.enableUTF8Print();  // Enable UTF8 support if needed
    u8g2.setBitmapMode(1);
    
    Serial.println("[INFO] Display initialized successfully!");

    // Set default font
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setFontDirection(0);

    screenInitialized = true;
    clearScreen();
    return true;
}

void DisplayModules::clearScreen()
{
    if (!screenInitialized) {
        Serial.println("[ERROR] Display not initialized, cannot clear screen.");
        return;
    }
    u8g2.clearBuffer();
    //u8g2.sendBuffer();
}

void DisplayModules::printString(String msg) {
    u8g2.print(msg);
}

void DisplayModules::drawBipmap(int x, int y, int w, int h, const uint8_t* icon, bool senddisplay) {
    u8g2.drawXBMP(x, y, w, h, icon);
    if (senddisplay) u8g2.sendBuffer();
}

float DisplayModules::calculateGraphScale(int16_t value) {
    if (value <= 0) return graph_scale;

    float targetScale = (float)(GRAPH_VERTICAL_LINE_LIM * 0.9f) / (float)value;

    if (targetScale > 1.0f) targetScale = 1.0f;
    if (targetScale < 0.1f) targetScale = 0.1f;

    return (graph_scale * 0.8f) + (targetScale * 0.2f);
}

void DisplayModules::addValueToGraph(int16_t value, int16_t* graph_array, int graph_array_size) {
    // Shift the array to the left
    for (int i = graph_array_size - 1; i > 0; i--) {
        graph_array[i] = graph_array[i - 1];
    }
    // Add the new value at the end
    graph_array[0] = value;
}

void DisplayModules::drawingGraph(int16_t* array, bool senddisplay) {
    drawingLine(0, 0, 0, 63);
    drawingLine(SCR_WIDTH - 1, 0, SCR_WIDTH - 1, 63);

    for (int count = 10; count < SCR_WIDTH - 1; count += 10) {
		drawingPixel(count, 0);
		drawingPixel(count, 63);
    }

    for (int i = 0; i < SCR_WIDTH; i++) {
        int yValue = (int)(array[i] * graph_scale);
        if (yValue < 1) yValue = 1;
        if (yValue > GRAPH_VERTICAL_LINE_LIM) yValue = GRAPH_VERTICAL_LINE_LIM;

        int yStart = SCR_HEIGHT - 1;
        int yEnd = SCR_HEIGHT - 1 - yValue;

        this->drawingLine(i - 1, yStart, i - 1, yEnd);
    }
    if (senddisplay) {
        u8g2.sendBuffer();
    }
}

float DisplayModules::graphScaleCheck(const int16_t array[SCR_WIDTH]) {
    int16_t maxValue = 0;
  
    // Iterate through the array to find the highest value
    for (int16_t i = 0; i < SCR_WIDTH; i++) {
      if (array[i] > maxValue) {
        maxValue = array[i];
      }
    }
  

    return this->calculateGraphScale(maxValue);

}

void DisplayModules::clearGraph(int16_t *array) {
    for (int i = 0; i < SCR_WIDTH; i++) {
        array[i] = 0;
    }
}


void DisplayModules::displayString(String msg, bool ln, bool senddisplay, int color)
{
    if (!screenInitialized) {
        Serial.println("[ERROR] Display not initialized, cannot display string.");
        return;
    }
    
    // U8g2 doesn't have direct print/println like Adafruit
    // We need to manage cursor position manually
    static int currentY = 12; // Start position (baseline)
    static int currentX = 0;
    
    if (ln) {
        // Check if message contains newlines
        if (msg.indexOf('\n') != -1) {
            this->drawMultiLineText(msg, currentX, currentY, 12);
            // Update Y position after multiline text
            int lineCount = 1;
            for (int i = 0; i < msg.length(); i++) {
                if (msg[i] == '\n') lineCount++;
            }
            currentY += lineCount * 12;
        } else {
            u8g2.drawStr(currentX, currentY, msg.c_str());
            currentY += 12; // Move to next line
        }
        
        currentX = 0; // Reset X position for new line
        
        if (currentY > SCR_HEIGHT - 12) {
            currentY = 12; // Reset if we reach bottom
        }
    } else {
        // For non-newline, print at current position and update X
        u8g2.drawStr(currentX, currentY, msg.c_str());
        currentX += u8g2.getStrWidth(msg.c_str());
        
        // Wrap to next line if needed
        if (currentX >= SCR_WIDTH - 10) {
            currentX = 0;
            currentY += 12;
            if (currentY > SCR_HEIGHT - 12) {
                currentY = 12;
            }
        }
    }
    
    if (senddisplay) {
        u8g2.sendBuffer();
    }
}

void DisplayModules::drawingCenterString(String msg, int y, bool senddisplay, int color)
{
    if (!screenInitialized) {
        Serial.println("[ERROR] Display not initialized, cannot draw centered text.");
        return;
    }
    
    // Calculate X position to center the text
    int x = (SCR_WIDTH - (msg.length() * 6)) / 2;
    
    u8g2.drawStr(x, y, msg.c_str());
    
    if (senddisplay) {
        u8g2.sendBuffer();
    }
}

void DisplayModules::drawingRect(int x, int y, int w, int h, bool fill_rect, int color)
{
    if (!screenInitialized) {
        Serial.println("[ERROR] Display not initialized, cannot draw rectangle.");
        return;
    }
    
    if (fill_rect) {
        u8g2.drawBox(x, y, w, h);
    } else {
        u8g2.drawFrame(x, y, w, h);
    }
    u8g2.sendBuffer();
}

void DisplayModules::drawingLine(int x1, int y1, int x2, int y2, bool sendDisplay)
{
    u8g2.drawLine(x1, y1, x2, y2);
    if (sendDisplay) u8g2.sendBuffer();
}

void DisplayModules::drawingPixel(int x, int y, bool sendDisplay) {
    u8g2.drawPixel(x, y);
    if (sendDisplay) u8g2.sendBuffer();
}

void DisplayModules::displayInvert(bool invert) 
{
    if (invert) {
        u8g2.sendF("c", 0xa7); // Invert display command
    } else {
        u8g2.sendF("c", 0xa6); // Normal display command
    }
}

void DisplayModules::displayStringwithCoordinates(String msg, int x, int y, bool senddisplay, int color)
{
    if (!screenInitialized) {
        Serial.println("[ERROR] Display not initialized, cannot display string with coordinates.");
        return;
    }
    
    u8g2.drawStr(x, y, msg.c_str());
    if (senddisplay) {
        u8g2.sendBuffer();
    }
}

void DisplayModules::displayBuffer()
{
    if (!screenInitialized) {
        Serial.println("[ERROR] Display not initialized, cannot display buffer.");
        return;
    }
    int y = 24; // Start at baseline position
    
    while (display_buffer->size() > 4) display_buffer->shift();

    for (int i = 0; i < display_buffer->size(); i++)
    {
        String msg = display_buffer->get(i);
        if (msg.length() > 0) {
            // Check if message contains newlines
            if (msg.indexOf('\n') != -1) {
                this->drawMultiLineText(msg, 0, y, 12);
                // Calculate how many lines this message took
                int lineCount = 1;
                for (int i = 0; i < msg.length(); i++) {
                    if (msg[i] == '\n') lineCount++;
                }
                y += lineCount * 12;
            } else {
                u8g2.drawStr(0, y, msg.c_str());
                y += 12;
            }
            
            //if (y > SCR_HEIGHT - 12) break; // Prevent overflow
        }
        else {
            Serial.println("[INFO] Empty message in display buffer, skipping...");
        }
    }
    u8g2.sendBuffer();
}

void DisplayModules::clearBuffer() {
    display_buffer->clear();
}

void DisplayModules::displayEvilPortalText(String username, String password) 
{
    String currentUsername = "u: " + username;
    String currentPassword = "p: " + password;

    this->clearScreen();
    
    // Display username với wrap text - bắt đầu từ dòng 1
    int nextY = this->drawWrappedText(currentUsername, 0, 12, 21);
    
    // Display password với wrap text - bắt đầu từ dòng tiếp theo
    this->drawWrappedText(currentPassword, 0, nextY + 3, 21); // +3 để có khoảng cách giữa username và password
    
    u8g2.sendBuffer();
}

void DisplayModules::setCursor(int x, int y) {
    u8g2.setCursor(x, y);
}

String DisplayModules::wrapText(String input, int lineWidth) 
{
    String result = "";
    for (int i = 0; i < input.length(); i++) {
        if (i > 0 && i % lineWidth == 0) {
            result += "\n";
        }
        result += input[i];
    }
    return result;
}

void DisplayModules::simpleStrikethrough(String text, int x, int y) 
{
    // Display text first
    u8g2.drawStr(x, y, text.c_str());
    
    // Draw strikethrough line
    int textWidth = u8g2.getStrWidth(text.c_str());
    u8g2.drawHLine(x, y - 2, textWidth); // Adjust y offset as needed
    
    u8g2.sendBuffer();
}

void DisplayModules::drawingVLine(int x, int y, int h, int color) 
{
    u8g2.drawVLine(x, y, h);
}

void DisplayModules::sendDisplay() 
{
    u8g2.sendBuffer();
}

/*
// Method to add text to display buffer
void DisplayModules::addToBuffer(String msg)
{
    if (display_buffer != nullptr) {
        display_buffer->add(msg);
    }
}
*/

// New method for proper text wrapping without spaces
int DisplayModules::drawWrappedText(String text, int x, int y, int charsPerLine)
{
    int currentY = y;
    int pos = 0;
    
    while (pos < text.length()) {
        String line = "";
        int remaining = text.length() - pos;
        int lineLength = (remaining < charsPerLine) ? remaining : charsPerLine;
        
        line = text.substring(pos, pos + lineLength);
        
        u8g2.drawStr(x, currentY, line.c_str());
        
        pos += lineLength;
        currentY += 12; // Move to next line
        
        // Prevent overflow
        if (currentY > SCR_HEIGHT - 12) break;
    }
    
    return currentY; // Return next available Y position
}

// Additional U8g2 specific methods
void DisplayModules::setFont(const uint8_t *font)
{
    u8g2.setFont(font);
}

int DisplayModules::getStrWidth(const char *s)
{
    return u8g2.getStrWidth(s);
}

void DisplayModules::drawMultiLineText(String text, int x, int y, int lineHeight)
{
    // First, wrap the text
    String wrappedText = this->wrapText(text, 21);
    
    int currentY = y;
    int startPos = 0;
    
    while (startPos < wrappedText.length()) {
        int endPos = wrappedText.indexOf('\n', startPos);
        if (endPos == -1) {
            // Last line
            String line = wrappedText.substring(startPos);
            if (line.length() > 0) {
                u8g2.drawStr(x, currentY, line.c_str());
            }
            break;
        }
        
        String line = wrappedText.substring(startPos, endPos);
        if (line.length() > 0) {
            u8g2.drawStr(x, currentY, line.c_str());
        }
        
        currentY += lineHeight;
        
        // Prevent overflow
        if (currentY > SCR_HEIGHT - lineHeight) break;
        
        startPos = endPos + 1;
    }
}
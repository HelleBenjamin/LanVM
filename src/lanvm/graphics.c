/*  
 * Lanskern ByteCode - A Virtual Machine & Assembler 
 * Copyright (c) 2025 Benjamin Helle
 *  
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or  
 * (at your option) any later version.  
 *  
 * This program is distributed in the hope that it will be useful,  
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the  
 * GNU General Public License for more details.  
 *  
 * You should have received a copy of the GNU General Public License  
 * along with this program. If not, see <https://www.gnu.org/licenses/>.  
 */
#include "../include/lanvm.h"

void langlClear(VM *vm) {
  memset(vm->framebuffer, 0, sizeof(vm->framebuffer));
}

void langlSetColor(VM *vm, uint8_t color) {
  uint8_t r = (color & 0xF0) >> 4;
  uint8_t g = (color & 0x0F);
  vm->currentColor = (r * 17) << 16 | (g * 17) << 8 | (r * 17) | 0xFF000000;
}

void langlPlot(VM *vm, int x, int y) {
  if (x >= 0 && x < vm->screenWidth && y >= 0 && y < vm->screenHeight) {
    vm->framebuffer[y * vm->screenWidth + x] = vm->currentColor;
  }
}

void langlLine(VM *vm, int x1, int y1, int x2, int y2) {
  int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
  int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1; 
  int err = (dx > dy ? dx : -dy) / 2, e2;

  while (true) {
    langlPlot(vm, x1, y1);
    if (x1 == x2 && y1 == y2) break;
    e2 = err;
    if (e2 > -dx) { err -= dy; x1 += sx; }
    if (e2 < dy) { err += dx; y1 += sy; }
  }
}

void langlRect(VM *vm, int x, int y, int w, int h) {
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      langlPlot(vm, x + i, y + j);
    }
  }
}


void langlRender(VM *vm) {
  glDrawPixels(vm->screenWidth, vm->screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, vm->framebuffer);
  glfwSwapBuffers(vm->window);
}

int langlInit(VM *vm) {
  vm->currentColor = 0x0000FFFF;
  if(glfwInit() != GLFW_TRUE){
    vm_exception(NULL, ERR_GRAPHICS, EXC_SEVERE, "Failed to initialize GLFW\nError: %s", glfwGetError(NULL));
    return 1;
  }
  vm->window = glfwCreateWindow(vm->screenWidth, vm->screenWidth, "LanVM Graphics", NULL, NULL);
  if(vm->window == NULL){
    vm_exception(NULL, ERR_GRAPHICS, EXC_SEVERE, "Failed to create GLFW window\nError: %s", glfwGetError(NULL));
    glfwTerminate();
    return 0;
  }
  vm->framebuffer = malloc(vm->screenWidth * vm->screenHeight * sizeof(uint32_t));
  if (!vm->framebuffer) vm_exception(vm, ERR_MALLOC, EXC_SEVERE, "Failed to allocate framebuffer\n");
  glfwMakeContextCurrent(vm->window);
  langlClear(vm);
  return 0;
}

void langlExit(VM *vm) {
  free(vm->framebuffer);
  glfwDestroyWindow(vm->window);
  glfwTerminate();
}
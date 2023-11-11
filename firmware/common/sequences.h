#pragma once
#include "color.h"

enum class LedChunkType {
  SetColor,
  Wait,
  Finish,
};

struct LedChunk {
  LedChunkType type;
  uint32_t time_ms;
  Color color;
};

const LedChunk lsqStart[] = {
    {LedChunkType::SetColor, 0, {255, 0, 0}},
    {LedChunkType::Wait, 207},
    {LedChunkType::SetColor, 0, {0, 255, 0}},
    {LedChunkType::Wait, 207},
    {LedChunkType::SetColor, 0, {0, 0, 255}},
    {LedChunkType::Wait, 207},
    {LedChunkType::SetColor, 0, {0, 0, 0}},
    {LedChunkType::Finish},
};

const LedChunk lsqFastBlink[] = {
    {LedChunkType::SetColor, 0, {255, 255, 255}},
    {LedChunkType::Wait, 100},
    {LedChunkType::SetColor, 0, {0, 0, 0}},
    {LedChunkType::Wait, 100},
    {LedChunkType::SetColor, 0, {255, 255, 255}},
    {LedChunkType::Wait, 100},
    {LedChunkType::SetColor, 0, {0, 0, 0}},
    {LedChunkType::Wait, 100},
    {LedChunkType::SetColor, 0, {255, 255, 255}},
    {LedChunkType::Wait, 100},
    {LedChunkType::SetColor, 0, {0, 0, 0}},
    {LedChunkType::Wait, 100},
    {LedChunkType::Finish},
};


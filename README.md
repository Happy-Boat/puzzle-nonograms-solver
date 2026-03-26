# puzzle-nonograms-solver

一个专门用于解决 [cn.puzzle-nonograms.com](https://cn.puzzle-nonograms.com) 上数织（Nonograms）谜题的求解器。

## 项目简介

本项目旨在自动求解数织（Nonograms）谜题。数织是一种经典的逻辑绘图游戏，玩家需要根据网格左侧和顶部的数字提示，推断出哪些格子需要填充，最终拼出一幅隐藏的像素画。

该求解器由两部分组成：
- **C++ 核心求解算法** (`solve.cpp`)：实现动态规划谜题求解。
- **Python 数据处理脚本** (`data.py`)：负责从网站获取谜题数据。

## 功能特点

- 针对 [cn.puzzle-nonograms.com](https://cn.puzzle-nonograms.com) 的谜题格式进行优化。
- 适合学习数织求解算法（如回溯法、行列推理等）的参考实现。

## 文件说明

| 文件 | 语言 | 说明 |
|------|------|------|
| `solve.cpp` | C++ | 核心求解器，实现数织谜题的主要逻辑。 |
| `data.py` | Python | 辅助脚本，用于爬取、解析谜题数据。 |

编译依赖: 

- boost(system locale) 
- Qt5 
- Qt5Pdfium
- OpenSSL

运行依赖:
- translate-shell(可选)
- tesseract
- tesseract-data-eng
- tesseract-data-chi-sim

测试环境:
- Manjaro Linux
- Clion 19.1.4
- Qt 5.12.3
- Boost 1.69.0

主要实现在application.h里
组件有QWidget与Brush两类, Brush可读取鼠标轨迹并修改图像
properties()存储全局属性并侦听变化

需在config.h里设定tesseract路径
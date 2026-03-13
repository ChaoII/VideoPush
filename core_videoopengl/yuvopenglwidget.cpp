#include "yuvopenglwidget.h"
#include "openglinclude.h"
#include <iostream>

YuvWidget::YuvWidget(QWidget *parent) : QOpenGLWidget(parent) {
    //Qt6 需要设置表面格式
    QSurfaceFormat format = this->format();
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    //启用多重采样抗锯齿 (4x MSAA)
    format.setSamples(4);
    this->setFormat(format);

    //GLSL3.0 版本后废弃了attribute/varying对应用in/out作为前置关键字
    QStringList list;
    //Qt6 Core Profile 需要使用 layout location
    list << "#version 330 core";
    list << "layout(location = 0) in vec4 vertexIn;";
    list << "layout(location = 1) in vec2 textureIn;";
    list << "out vec2 textureOut;";
    list << "";
    list << "void main(void)";
    list << "{";
    list << "  gl_Position = vertexIn;";
    list << "  textureOut = textureIn;";
    list << "}";
    shaderVert = list.join("\n");

    list.clear();
    initFragment(list);
    list << "#version 330 core";
    list << "in vec2 textureOut;";
    list << "uniform sampler2D textureY;";
    list << "uniform sampler2D textureU;";
    list << "uniform sampler2D textureV;";
    list << "out vec4 fragColor;";
    list << "";
    list << "void main(void)";
    list << "{";
    list << "  vec3 yuv;";
    list << "  vec3 rgb;";
    list << "  yuv.r = texture2D(textureY, textureOut).r;";
    list << "  yuv.g = texture2D(textureU, textureOut).r - 0.5;";
    list << "  yuv.b = texture2D(textureV, textureOut).r - 0.5;";
    list << "  rgb = mat3(1.0, 1.0, 1.0, 0.0, -0.138, 1.816, 1.540, -0.459, 0.0) * yuv;";
    list << "  fragColor = vec4(rgb, 1.0);";
    list << "}";
    shaderFrag = list.join("\n");

    yuyv = false;
    this->initData();

    //关联定时器读取文件
    connect(&timer, SIGNAL(timeout()), this, SLOT(read()));
}

YuvWidget::~YuvWidget() {
    makeCurrent();
    if (m_vao.isCreated()) {
        m_vao.destroy();
    }
    doneCurrent();
}

void YuvWidget::setYuyv(bool yuyv) {
    this->yuyv = yuyv;
}

void YuvWidget::clear() {
    this->initData();
    this->update();
}

void YuvWidget::setFrameSize(int width, int height) {
    this->width = width;
    this->height = height;
}

void YuvWidget::updateTextures(quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY, quint32 linesizeU,
                               quint32 linesizeV) {
    this->dataY = dataY;
    this->dataU = dataU;
    this->dataV = dataV;
    this->lineSizeY = linesizeY;
    this->lineSizeU = linesizeU;
    this->lineSizeV = linesizeV;
    //确保OpenGL上下文已初始化
    if (m_vao.isCreated()) {
        this->update();
    }
}

void YuvWidget::updateFrame(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY,
                            quint32 linesizeU, quint32 linesizeV) {
    this->setFrameSize(width, height);
    this->updateTextures(dataY, dataU, dataV, linesizeY, linesizeU, linesizeV);
}

void YuvWidget::initializeGL() {
    initializeOpenGLFunctions();
    glDisable(GL_DEPTH_TEST);

    //Qt6 Core Profile 需要 VAO
    m_vao.create();

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    //创建 VBO，顶点和纹理坐标交错存储
    //每个顶点: x, y (2 floats), tx, ty (2 floats) = 4 floats = 16 bytes
    //总共4个顶点
    static const GLfloat vertices[] = {
        //顶点坐标       //纹理坐标
        -1.0f, -1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,   0.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 0.0f
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));

    //设置顶点坐标属性 (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    //设置纹理坐标属性 (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    //初始化 shader
    this->initShader();
    //初始化 textures
    this->initTextures();
    //初始化颜色
    this->initColor();
}

void YuvWidget::paintGL() {
    if (!dataY || width == 0 || height == 0) {
        this->initColor();
        return;
    }

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    program.bind();

    //Qt6 Core Profile 使用 GL_RED 替代 GL_LUMINANCE
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, lineSizeY);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, dataY);
    glUniform1i(textureUniformY, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU);
    quint32 uWidth = width >> 1;
    quint32 uHeight = yuyv ? height : height >> 1;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, lineSizeU);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uWidth, uHeight, 0, GL_RED, GL_UNSIGNED_BYTE, dataU);
    glUniform1i(textureUniformU, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, lineSizeV);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uWidth, uHeight, 0, GL_RED, GL_UNSIGNED_BYTE, dataV);
    glUniform1i(textureUniformV, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void YuvWidget::initData() {
    width = height = 0;
    dataY = dataU = dataV = nullptr;
    lineSizeY = lineSizeU = lineSizeV = 0;
}

void YuvWidget::initColor() {
    //取画板背景颜色
    QColor color = palette().window().color();
    //设置背景清理色
    glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    //清理颜色背景
    glClear(GL_COLOR_BUFFER_BIT);
}

void YuvWidget::initShader() {
    //加载顶点和片元脚本
    program.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderVert);
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderFrag);

    //编译 shader
    if (!program.link()) {
        qDebug() << "Shader link error:" << program.log();
    }

    program.bind();

    //从 shader 获取地址
    textureUniformY = program.uniformLocation("textureY");
    textureUniformU = program.uniformLocation("textureU");
    textureUniformV = program.uniformLocation("textureV");
}

void YuvWidget::initTextures() {
    //创建纹理
    glGenTextures(1, &textureY);
    glBindTexture(GL_TEXTURE_2D, textureY);
    this->initParamete();

    glGenTextures(1, &textureU);
    glBindTexture(GL_TEXTURE_2D, textureU);
    this->initParamete();

    glGenTextures(1, &textureV);
    glBindTexture(GL_TEXTURE_2D, textureV);
    this->initParamete();
}

void YuvWidget::initParamete() {
    //纹理过滤
    //GL_LINEAR: 线性插值过滤,获取坐标点附近4个像素的加权平均值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //纹理贴图
    //GL_CLAMP_TO_EDGE: 超出纹理范围的坐标被截取成0和1,形成纹理边缘延伸的效果
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void YuvWidget::deleteTextures() {
    glDeleteTextures(1, &textureY);
    glDeleteTextures(1, &textureU);
    glDeleteTextures(1, &textureV);
}

void YuvWidget::read() {
    qint64 len = (width * height * 3) >> 1;
    if (file.read((char *) dataY, len)) {
        this->update();
    } else {
        timer.stop();
        emit playFinish();
    }
}

void YuvWidget::play(const QString &fileName, int frameRate) {
    //停止定时器并关闭文件
    if (timer.isActive()) {
        timer.stop();
    }
    if (file.isOpen()) {
        file.close();
    }

    file.setFileName(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    //初始化对应数据指针位置
    dataY = new quint8[(width * height * 3) >> 1];
    dataU = dataY + (width * height);
    dataV = dataU + ((width * height) >> 2);

    //启动定时器读取文件数据
    timer.start(1000 / frameRate);
}

void YuvWidget::stop() {
    //停止定时器并关闭文件
    if (timer.isActive()) {
        timer.stop();
    }
    if (file.isOpen()) {
        file.close();
    }

    this->clear();
    emit playFinish();
}

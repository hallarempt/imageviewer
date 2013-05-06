#include "DisplayWidget.h"

#include <QGLBuffer>
#include <QGLShaderProgram>
#include <QGLBuffer>
#include <QGLFramebufferObject>
#include <QGLPixelBuffer>
#include <QGLFormat>
#include <QMessageBox>
#include <QImage>
#include <QColor>
#include <QWheelEvent>

DisplayWidget::DisplayWidget(QGLWidget *context, QWidget *parent)
    : QGLWidget(parent, context)
    , m_shareWidget(context)
    , m_checkerBuffer(0)
    , m_compositeSrc(0)
    , m_compositeDst(0)
    , m_projection(0)
    , m_rotationAngle(0.0)
    , m_zoom(0.5)
    , m_documentOffset(QPointF(0.0, 0.0))
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
}

DisplayWidget::~DisplayWidget()
{
    delete m_displayShader;
    delete m_compositeOpShader;
    delete m_colorCorrectionShader;

    foreach(GLuint texture, m_layers) {
        glDeleteTextures(1, &texture);
    }


    m_layers.clear();
}

void DisplayWidget::addLayer(QImage image)
{
    qDebug() << image.size();
    m_images << image;
    GLuint texture = const_cast<QGLContext*>(QGLContext::currentContext())->bindTexture(image);
    m_layers << texture;
}

void DisplayWidget::initializeGL()
{
    createDisplayShader();
    createMesh();
    createCompositeOpShader();
    createColorCorrectionShader();
}

void DisplayWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);

    // clear the whole view to the gray color chosen by the user for the canvas border

    // for the checkers
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_displayShader->bind();

    QMatrix4x4 model;
    //model.scale(1, 1);
    m_displayShader->setUniformValue(m_modelMatrixLocation, model);

    //Set view/projection matrices
    QMatrix4x4 view;
    m_displayShader->setUniformValue(m_viewMatrixLocation, view);

    QMatrix4x4 proj;
    proj.ortho(0, 1, 0, 1, -1, 1);
    m_displayShader->setUniformValue(m_projectionMatrixLocation, proj);

    //Setup the geometry for rendering
    m_vertexBuffer->bind();
    m_indexBuffer->bind();

    m_displayShader->setAttributeBuffer(m_vertexLocation, GL_FLOAT, 0, 3);
    m_displayShader->enableAttributeArray(m_vertexLocation);
    m_displayShader->setAttributeBuffer(m_uv0Location, GL_FLOAT, 12 * sizeof(float), 2);
    m_displayShader->enableAttributeArray(m_uv0Location);
    m_displayShader->setUniformValue(m_texture0Location, 0);
    m_displayShader->setUniformValue(m_textureScaleLocation, QVector2D(1.0f, 1.0f));

    // render checkers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_checkerBuffer->texture());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    m_displayShader->release();

    if (m_projection) {
        // render projection with color correction
        m_colorCorrectionShader->bind();

        QVector3D imageSize(m_imageSize.width(), m_imageSize.height(), 0.f);

        model = QMatrix4x4();
        // XXX: fix rotation
        // model.translate(imageSize.x() / 2, -imageSize.y() / 2); // to center
        // model.rotate(m_rotationAngle, 0.f, 0.f, -1.f); // rotate
        // model.translate(-imageSize.x() / 2, imageSize.y() / 2); // return to top left
        model.scale(imageSize * m_zoom);

        m_colorCorrectionShader->setUniformValue("modelMatrix", model);

        //Set view/projection matrices
        view = QMatrix4x4();
//        QPointF translation = m_documentOffset;
//        view.translate(-translation.x(), translation.y());
//        qreal scale = m_zoom;
//        view.scale(scale, scale);
        m_colorCorrectionShader->setUniformValue("viewMatrix", view);

        proj = QMatrix4x4();
        // -10, -10 = depth range
        proj.ortho(0, width(), 0, height(), -10, 10);
        m_colorCorrectionShader->setUniformValue("projectionMatrix", proj);

        m_colorCorrectionShader->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
        m_colorCorrectionShader->enableAttributeArray("vertex");
        m_colorCorrectionShader->setAttributeBuffer("inputTextureCoordinate", GL_FLOAT, 12 * sizeof(float), 2);
        m_colorCorrectionShader->enableAttributeArray("inputTextureCoordinate");
        m_colorCorrectionShader->setUniformValue("texture0", 0);

        glBindTexture(GL_TEXTURE_2D, m_projection->texture());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    m_indexBuffer->release();
    m_vertexBuffer->release();

    m_colorCorrectionShader->release();

}

void DisplayWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    createCheckerTexture(w, h);
    updateProjection();
}

void DisplayWidget::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0)
        m_zoom += 0.5;
    else
        m_zoom -= 0.5;
    updateProjection();
}

// dst is painted on src, dst == top, output to dst
void DisplayWidget::composeTwo(QGLFramebufferObject *src, QGLFramebufferObject *dst, GLuint texture, const QString &/*compositeOp*/)
{

    // from now on everything goes to this framebuffer object
    dst->bind();

    m_compositeOpShader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src->texture());
    m_compositeOpShader->setUniformValue("inputTextureBot", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);
    m_compositeOpShader->setUniformValue("inputTextureTop", 1);

    m_compositeOpShader->setUniformValue("alphaTop", 1.0f);

    //Set mode matrix;
    QMatrix4x4 model;
    m_compositeOpShader->setUniformValue("modelMatrix", model);

    //Set view matrix
    QMatrix4x4 view;
    m_compositeOpShader->setUniformValue("viewMatrix", view);

    //Set projection matrix
    QMatrix4x4 proj;
    // depth: default -1, 1
    proj.ortho(0, 1, 0, 1, -1, 1);
    m_compositeOpShader->setUniformValue("projectionMatrix", proj);

    //Setup the geometry for rendering
    m_vertexBuffer->bind();
    m_indexBuffer->bind();

    m_compositeOpShader->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
    m_compositeOpShader->enableAttributeArray("vertex");

    m_compositeOpShader->setAttributeBuffer("inputTextureCoordinate", GL_FLOAT, 12 * sizeof(float), 2);
    m_compositeOpShader->enableAttributeArray("inputTextureCoordinate");

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    m_compositeOpShader->disableAttributeArray("inputTextureCoordinate");
    m_compositeOpShader->disableAttributeArray("vertex");

    m_indexBuffer->release();
    m_vertexBuffer->release();
    m_displayShader->release();
\
    // from now on everything goes to the window framebuffer again
    dst->release();
}

void DisplayWidget::updateProjection()
{
    int w = 0;
    int h = 0;

    foreach (const QImage &img, m_images) {
        w = qMax(img.width(), w);
        h = qMax(img.height(), h);
    }
    m_imageSize = QSize(w, h);
    glViewport(0, 0, w, h);

    delete m_compositeSrc;
    delete m_compositeDst;
    m_compositeSrc = new QGLFramebufferObject(w, h);
    m_compositeDst = new QGLFramebufferObject(w, h);

    m_compositeSrc->bind();
    glClear(GL_COLOR_BUFFER_BIT);

    m_compositeDst->bind();
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the layers onto the buffer
    for (int i = 0; i < m_layers.size(); ++i) {
        composeTwo(m_compositeSrc, m_compositeDst, m_layers[i], "COMPOSITE_OVER");
        m_projection = m_compositeDst;
        qSwap(m_compositeSrc, m_compositeDst);
    }


    m_compositeSrc->toImage().save("src.png");
    m_compositeDst->toImage().save("dst.png");

    glViewport(0, 0, width(), height());
    // kick paintGL
    update();
}


void DisplayWidget::createMesh()
{
    m_vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
    m_vertexBuffer->create();
    m_vertexBuffer->bind();

    QVector<float> vertices;
    /*
     *  0.0, 1.0  ---- 1.0, 1.0
     *     |              |
     *     |              |
     *  0.0, 0.0  ---- 1.0, 0.0
     */
    vertices << 0.0f << 0.0f << 0.0f;
    vertices << 0.0f << 1.0f << 0.0f;
    vertices << 1.0f << 0.0f << 0.0f;
    vertices << 1.0f << 1.0f << 0.0f;
    int vertSize = sizeof(float) * vertices.count();

    // coordinates to convert vertex points to a position in the texture. Follows order of corner
    // points in vertices
    QVector<float> uvs;
    uvs << 0.f << 0.f;
    uvs << 0.f << 1.f;
    uvs << 1.f << 0.f;
    uvs << 1.f << 1.f;
    int uvSize = sizeof(float) * uvs.count();

    m_vertexBuffer->allocate(vertSize + uvSize);
    m_vertexBuffer->write(0, reinterpret_cast<void*>(vertices.data()), vertSize);
    m_vertexBuffer->write(vertSize, reinterpret_cast<void*>(uvs.data()), uvSize);
    m_vertexBuffer->release();

    m_indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
    m_indexBuffer->create();
    m_indexBuffer->bind();

    QVector<uint> indices;
    // determines where opengl looks for vertex data. create two clockwise triangles from
    // the points.
    /*
     *  1->-3
     *  |\  |
     *  ^ \ v
     *  |  \|
     *  0...2
     */
    indices << 0 << 1 << 2 << 1 << 3 << 2;
    m_indexBuffer->allocate(reinterpret_cast<void*>(indices.data()), indices.size() * sizeof(uint));
    m_indexBuffer->release();
}


void DisplayWidget::createCheckerTexture(int w, int h)
{
    int size = 64;
    QColor color = Qt::darkGray;

    int halfSize = size / 2;

    if(m_checkerBuffer != 0) {
        delete m_checkerBuffer;
    }

    m_checkerBuffer = new QGLFramebufferObject(w, h, QGLFramebufferObject::CombinedDepthStencil);
    m_checkerBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT);

    QImage checkers(size, size, QImage::Format_ARGB32);
    QPainter painter;
    painter.begin(&checkers);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::lightGray));
    painter.drawRect(0, 0, size, size);
    painter.setBrush(QBrush(color));
    painter.drawRect(0, 0, halfSize, halfSize);
    painter.drawRect(halfSize, halfSize, halfSize, halfSize);
    painter.end();

    QPainter gc(m_checkerBuffer);
    gc.fillRect(0, 0, w, h, checkers);
    gc.end();

    m_checkerBuffer->release();
}


void DisplayWidget::createDisplayShader()
{
    m_displayShader = new QGLShaderProgram();
    m_displayShader->addShaderFromSourceFile(QGLShader::Vertex, ":/gl2.vert");
    m_displayShader->addShaderFromSourceFile(QGLShader::Fragment, ":/gl2.frag");

    bool r = m_displayShader->link();
    if (!r) {
        qDebug() << "failed linking display shader" << glGetError();
    }

    m_modelMatrixLocation      = m_displayShader->uniformLocation("modelMatrix");
    m_viewMatrixLocation       = m_displayShader->uniformLocation("viewMatrix");
    m_projectionMatrixLocation = m_displayShader->uniformLocation("projectionMatrix");

    m_vertexLocation           = m_displayShader->attributeLocation("vertex");
    m_uv0Location              = m_displayShader->attributeLocation("uv0");

    m_texture0Location         = m_displayShader->uniformLocation("texture0");
    m_textureScaleLocation     = m_displayShader->uniformLocation("textureScale");
}



void DisplayWidget::createCompositeOpShader()
{
    m_compositeOpShader = new QGLShaderProgram(this);
    m_compositeOpShader->addShaderFromSourceFile(QGLShader::Vertex, ":/composite.vert");
    m_compositeOpShader->addShaderFromSourceFile(QGLShader::Fragment, ":/composite_over.frag");
    bool r = m_compositeOpShader->link();
    if (!r) {
        qDebug() << "failed linking composite op shader" << glGetError();
    }


}

void DisplayWidget::createColorCorrectionShader()
{
    m_colorCorrectionShader = new QGLShaderProgram(this);
    m_colorCorrectionShader->addShaderFromSourceFile(QGLShader::Vertex, ":/composite.vert");
    m_colorCorrectionShader->addShaderFromSourceFile(QGLShader::Fragment, ":/lut.frag");
    bool r = m_colorCorrectionShader->link();
    if (!r) {
        qDebug() << "failed linking lut shader" << glGetError();
    }


}

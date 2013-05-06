#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QGLWidget>
#include <QImage>
#include <QList>

class QGLShaderProgram;
class QGLBuffer;
class QGLFramebufferObject;
class QGLPixelBuffer;
class QGLFormat;


/**
 * QImage: source, randomly updated to force updates
 * 1. create texture
 * 2. apply color correction
 * 3. paint over checkers
 * 4. paint tool decorations
 * 5. paint on screen
 */
class DisplayWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit DisplayWidget(QGLWidget *context, QWidget *parent = 0);
    virtual ~DisplayWidget();

    void addLayer(QImage image);
    
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w, int h);

protected:

    virtual void wheelEvent(QWheelEvent *event);

signals:
    
public slots:

    void updateProjection();

private:


    void composeTwo(QGLFramebufferObject *src, QGLFramebufferObject *dst, GLuint texture, const QString &compositeOp);

    void composeLayers();
    void createCheckerTexture(int w, int h);
    void createImageTexture();
    void createMesh();

    void createDisplayShader();
    QGLShaderProgram *m_displayShader;

    int m_modelMatrixLocation;
    int m_viewMatrixLocation;
    int m_projectionMatrixLocation;
    int m_texture0Location;
    int m_textureScaleLocation;
    int m_vertexLocation;
    int m_uv0Location;

    void createCompositeOpShader();
    QGLShaderProgram *m_compositeOpShader;

    void createColorCorrectionShader();
    QGLShaderProgram *m_colorCorrectionShader;

    QGLWidget *m_shareWidget;

    QVector<QImage> m_images;
    QVector<GLuint> m_layers;

    QGLBuffer *m_vertexBuffer;
    QGLBuffer *m_indexBuffer;

    // when the image can be updated runtime, then we need to have a front
    // and back buffer and swap them so the paintGL always has a complete
    // projection, not necessary now
    QGLFramebufferObject *m_compositeSrc;
    QGLFramebufferObject *m_compositeDst;
    QGLFramebufferObject *m_projection;
    QGLFramebufferObject *m_checkerBuffer;

    qreal m_rotationAngle;
    qreal m_zoom;
    QPointF m_documentOffset;
    QSize m_imageSize;

};

#endif // DISPLAYWIDGET_H

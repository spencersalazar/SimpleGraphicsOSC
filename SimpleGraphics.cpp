/******************************************************************************

 @File         OGLES2HelloTriangle_NullWS.cpp

 @Title        OpenGL ES 2.0 Hello Triangle Tutorial

 @Copyright    Copyright (C)  Imagination Technologies Limited.

 @Platform     .

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenGL ES
               2.0, use it for drawing a triangle and terminate it.

******************************************************************************/
#include <stdio.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "stgl.h"
#include "st.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <map>
#include "CircularBuffer.h"
#include "STTexture.h"
#include "STImage.h"

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

#ifdef RASPBERRY_PI
#include  "bcm_host.h"
#endif

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY    0


struct SGMessage
{
    enum Type
    {
        LINE,
        TRIANGLE,
        RECT,
        ELLIPSE,
        IMAGE,
        TEXT,
        REMOVE,
        
        POSITION,
        SIZE,
        COLOR,
        RED,
        GREEN,
        BLUE,
        ALPHA,
    };
    
    Type type;
    std::string objectId;
    
    STPoint2 position;
    STPoint2 size;
    STPoint2 vertex3;
    STColor4f color;
    std::string str;
};

class SGObject
{
public:
    SGObject()
    {
        program = 0;
        geo = NULL;
        numVertex = 0;
        vbo = 0;
        
        glGenBuffers(1, &vbo);
    }
    
    virtual ~SGObject()
    {
        glDeleteBuffers(1, &vbo);
    }
    
    void setShaderProgram(GLuint p)
    {
        program = p;
    }

    virtual void processMessage(const SGMessage &msg)
    {
        switch(msg.type)
        {
            case SGMessage::COLOR:
            case SGMessage::RECT:
            case SGMessage::ELLIPSE:
            case SGMessage::LINE:
            case SGMessage::TRIANGLE:
            case SGMessage::IMAGE:
            case SGMessage::TEXT:
            {
                color = msg.color;
            }
            break;
            case SGMessage::RED:
            {
                color.r = msg.color.r;
            }
            break;
            case SGMessage::GREEN:
            {
                color.g = msg.color.g;
            }
            break;
            case SGMessage::BLUE:
            {
                color.b = msg.color.b;
            }
            break;
            case SGMessage::ALPHA:
            {
                color.a = msg.color.a;
            }
            break;
            default:
            break;
        }
    }

    virtual void render()
    {
        if(numVertex == 0 || geo == NULL) return;
        // set color
        int location = glGetUniformLocation(program, "color");
        glUniform4f(location, color.r, color.g, color.b, color.a);
        location = glGetUniformLocation(program, "texOffset");
        glUniform4f(location, 1, 1, 1, 1);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, numVertex * (sizeof(GLfloat) * 2), geo, GL_STATIC_DRAW);
        glEnableVertexAttribArray(VERTEX_ARRAY);
        glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        GLuint textureUniform = glGetUniformLocation(program, "tex");
        glUniform1i(textureUniform, 0);
        
        glDrawArrays(GL_TRIANGLES, 0, numVertex);
    }
    
    const std::string &id() { return m_id; }

    static int SCREEN_WIDTH;
    static int SCREEN_HEIGHT;

protected:

    GLuint vbo;
    GLfloat *geo;
    int numVertex;
    STColor4f color;
    GLuint program;
        
private:
    std::string m_id;
};

int SGObject::SCREEN_WIDTH = 0;
int SGObject::SCREEN_HEIGHT = 0;

class SGRectangle : public SGObject
{
public:
    SGRectangle(float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;
        numVertex = 6;
        geo = new GLfloat[numVertex*2];
        setDimensions(x, y, width, height);
    }
    
    virtual ~SGRectangle()
    {
        delete[] geo;
        geo = NULL;
        numVertex = 0;
    }
    
    virtual void processMessage(const SGMessage &msg)
    {
        SGObject::processMessage(msg);
        
        switch(msg.type)
        {
            case SGMessage::RECT:
                setDimensions(msg.position.x, msg.position.y, 
                    msg.size.x, msg.size.y);
            break;
            case SGMessage::POSITION:
                setPosition(msg.position.x, msg.position.y);
            break;
            case SGMessage::SIZE:
                setPosition(msg.size.x, msg.size.y);
            break;
            default:
            break;
        }
    }
    
    void setDimensions(float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;
        
        float width_2 = width/2.0;
        float height_2 = height/2.0;
        
        geo[0]  = x-width_2; geo[1]  = y-height_2;
        geo[2]  = x+width_2; geo[3]  = y-height_2;
        geo[4]  = x-width_2; geo[5]  = y+height_2;
        geo[6]  = x+width_2; geo[7]  = y-height_2;
        geo[8]  = x-width_2; geo[9]  = y+height_2;
        geo[10] = x+width_2; geo[11] = y+height_2;
    }
    
    void setPosition(float _x, float _y)
    {
        x = _x; y = _y;
        
        float width_2 = width/2.0;
        float height_2 = height/2.0;
        
        geo[0]  = x-width_2; geo[1]  = y-height_2;
        geo[2]  = x+width_2; geo[3]  = y-height_2;
        geo[4]  = x-width_2; geo[5]  = y+height_2;
        geo[6]  = x+width_2; geo[7]  = y-height_2;
        geo[8]  = x-width_2; geo[9]  = y+height_2;
        geo[10] = x+width_2; geo[11] = y+height_2;
    }
    
    void setSize(float _width, float _height)
    {
        width = _width; height = _height;
        
        float width_2 = width/2.0;
        float height_2 = height/2.0;
        
        geo[0]  = x-width_2; geo[1]  = y-height_2;
        geo[2]  = x+width_2; geo[3]  = y-height_2;
        geo[4]  = x-width_2; geo[5]  = y+height_2;
        geo[6]  = x+width_2; geo[7]  = y-height_2;
        geo[8]  = x-width_2; geo[9]  = y+height_2;
        geo[10] = x+width_2; geo[11] = y+height_2;
    }
    
protected:
    float x, y, width, height;
};

class SGEllipse : public SGObject
{
public:
    SGEllipse(float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;
        tris = 80;
        numVertex = tris*3;
        geo = new GLfloat[tris*3*2];
        setDimensions(x, y, width, height);
    }
    
    virtual ~SGEllipse()
    {
        delete[] geo;
        geo = NULL;
        numVertex = 0;
    }
    
    virtual void processMessage(const SGMessage &msg)
    {
        SGObject::processMessage(msg);
        
        switch(msg.type)
        {
            case SGMessage::ELLIPSE:
                setDimensions(msg.position.x, msg.position.y, 
                    msg.size.x, msg.size.y);
            break;
            case SGMessage::POSITION:
                setPosition(msg.position.x, msg.position.y);
            break;
            case SGMessage::SIZE:
                setPosition(msg.size.x, msg.size.y);
            break;
            default:
            break;
        }
    }
    
    void setDimensions(float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;

        for(int i = 0; i < tris; i++)
        {
            float theta = ((float) i)/tris*2*M_PI;
            float theta_plus_one = ((float) i+1)/tris*2*M_PI;
            geo[i*6] = x;
            geo[i*6+1] = y;
            geo[i*6+2] = x+width*0.5f*cosf(theta);
            geo[i*6+3] = y+height*0.5f*sinf(theta);
            geo[i*6+4] = x+width*0.5f*cosf(theta_plus_one);
            geo[i*6+5] = y+height*0.5f*sinf(theta_plus_one);
        }
    }
    
    void setPosition(float _x, float _y)
    {
        x = _x; y = _y;

        for(int i = 0; i < tris; i++)
        {
            float theta = ((float) i)/tris*2*M_PI;
            float theta_plus_one = ((float) i+1)/tris*2*M_PI;
            geo[i*6] = x;
            geo[i*6+1] = y;
            geo[i*6+2] = x+width*0.5f*cosf(theta);
            geo[i*6+3] = y+height*0.5f*sinf(theta);
            geo[i*6+4] = x+width*0.5f*cosf(theta_plus_one);
            geo[i*6+5] = y+height*0.5f*sinf(theta_plus_one);
        }
    }
    
    void setSize(float _width, float _height)
    {
        width = _width; height = _height;

        for(int i = 0; i < tris; i++)
        {
            float theta = ((float) i)/tris*2*M_PI;
            float theta_plus_one = ((float) i+1)/tris*2*M_PI;
            geo[i*6] = x;
            geo[i*6+1] = y;
            geo[i*6+2] = x+width*0.5f*cosf(theta);
            geo[i*6+3] = y+height*0.5f*sinf(theta);
            geo[i*6+4] = x+width*0.5f*cosf(theta_plus_one);
            geo[i*6+5] = y+height*0.5f*sinf(theta_plus_one);
        }
    }
    
protected:
    int tris;
    float x, y, width, height;
};

class SGImage : public SGObject
{
public:
    SGImage(const std::string &imageFile, float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;
        numVertex = 6;
        geo = new GLfloat[2*numVertex*2];
        
        setDimensions(x, y, width, height);
        
        uv = geo + 2*numVertex;
        
        uv[0]  = 0; uv[1]  = 0;
        uv[2]  = 1; uv[3]  = 0;
        uv[4]  = 0; uv[5]  = 1;
        uv[6]  = 1; uv[7]  = 0;
        uv[8]  = 0; uv[9]  = 1;
        uv[10] = 1; uv[11] = 1;
        
        image = new STImage(imageFile.c_str());
        texture = new STTexture(image);
    }
    
    virtual ~SGImage()
    {
        delete[] geo;
        geo = NULL;
        numVertex = 0;
        delete texture;
        delete image;
    }
    
    virtual void processMessage(const SGMessage &msg)
    {
        SGObject::processMessage(msg);
        
        switch(msg.type)
        {
            case SGMessage::IMAGE:
                setDimensions(msg.position.x, msg.position.y, 
                    msg.size.x, msg.size.y);
            break;
            case SGMessage::POSITION:
                setPosition(msg.position.x, msg.position.y);
            break;
            case SGMessage::SIZE:
                setPosition(msg.size.x, msg.size.y);
            break;
            default:
            break;
        }
    }
    
    virtual void render()
    {
        if(numVertex == 0 || geo == NULL) return;
        
        GLuint texCoordSlot = glGetAttribLocation(program, "texCoordIn");
        glEnableVertexAttribArray(texCoordSlot);
        GLuint textureUniform = glGetUniformLocation(program, "tex");
        
        // set color
        int location = glGetUniformLocation(program, "color");
        glUniform4f(location, color.r, color.g, color.b, color.a);
        location = glGetUniformLocation(program, "texOffset");
        glUniform4f(location, 0, 0, 0, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, numVertex * 2 * (sizeof(GLfloat) * 2), 
            geo, GL_STATIC_DRAW);
        glEnableVertexAttribArray(VERTEX_ARRAY);
        glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
        glVertexAttribPointer(texCoordSlot, 2, GL_FLOAT, GL_FALSE, 0, 
            (GLvoid*) (sizeof(GLfloat)*numVertex*2));
        
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        texture->Bind();
        glUniform1i(textureUniform, 0);
        
        glDrawArrays(GL_TRIANGLES, 0, numVertex);
        
        texture->UnBind();
        glDisableVertexAttribArray(texCoordSlot);
    }
    
    void setDimensions(float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;
        
        float width_2 = width/2.0;
        float height_2 = height/2.0;
        
        geo[0]  = x-width_2; geo[1]  = y-height_2;
        geo[2]  = x+width_2; geo[3]  = y-height_2;
        geo[4]  = x-width_2; geo[5]  = y+height_2;
        geo[6]  = x+width_2; geo[7]  = y-height_2;
        geo[8]  = x-width_2; geo[9]  = y+height_2;
        geo[10] = x+width_2; geo[11] = y+height_2;
    }
    
    void setPosition(float _x, float _y)
    {
        x = _x; y = _y;
        
        float width_2 = width/2.0;
        float height_2 = height/2.0;
        
        geo[0]  = x-width_2; geo[1]  = y-height_2;
        geo[2]  = x+width_2; geo[3]  = y-height_2;
        geo[4]  = x-width_2; geo[5]  = y+height_2;
        geo[6]  = x+width_2; geo[7]  = y-height_2;
        geo[8]  = x-width_2; geo[9]  = y+height_2;
        geo[10] = x+width_2; geo[11] = y+height_2;
    }
    
    void setSize(float _width, float _height)
    {
        width = _width; height = _height;
        
        float width_2 = width/2.0;
        float height_2 = height/2.0;
        
        geo[0]  = x-width_2; geo[1]  = y-height_2;
        geo[2]  = x+width_2; geo[3]  = y-height_2;
        geo[4]  = x-width_2; geo[5]  = y+height_2;
        geo[6]  = x+width_2; geo[7]  = y-height_2;
        geo[8]  = x-width_2; geo[9]  = y+height_2;
        geo[10] = x+width_2; geo[11] = y+height_2;
    }
    
protected:
    GLfloat *uv;
    float x, y, width, height;
    STImage * image;
    STTexture * texture;
};


class SGLine : public SGObject
{
public:
    SGLine(float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;
        numVertex = 2;
        geo = new GLfloat[numVertex*2];
        setDimensions(x, y, width, height);
    }
    
    virtual ~SGLine()
    {
        delete[] geo;
        geo = NULL;
        numVertex = 0;
    }
    
    virtual void render()
    {
        if(numVertex == 0 || geo == NULL) return;
        // set color
        int location = glGetUniformLocation(program, "color");
        glUniform4f(location, color.r, color.g, color.b, color.a);
        location = glGetUniformLocation(program, "texOffset");
        glUniform4f(location, 1, 1, 1, 1);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, numVertex * (sizeof(GLfloat) * 2), geo, GL_STATIC_DRAW);
        glEnableVertexAttribArray(VERTEX_ARRAY);
        glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        GLuint textureUniform = glGetUniformLocation(program, "tex");
        glUniform1i(textureUniform, 0);
        
        glDrawArrays(GL_LINES, 0, numVertex);
    }
    
    virtual void processMessage(const SGMessage &msg)
    {
        SGObject::processMessage(msg);
        
        switch(msg.type)
        {
            case SGMessage::LINE:
                setDimensions(msg.position.x, msg.position.y, 
                    msg.size.x, msg.size.y);
            break;
            case SGMessage::POSITION:
                setPosition(msg.position.x, msg.position.y);
            break;
            case SGMessage::SIZE:
                setPosition(msg.size.x, msg.size.y);
            break;
            default:
            break;
        }
    }
    
    void setDimensions(float _x, float _y, float _width, float _height)
    {
        x = _x; y = _y;
        width = _width; height = _height;

        geo[0] = x;
        geo[1] = y;
        geo[2] = width;
        geo[3] = height;
    }
    
    void setPosition(float _x, float _y)
    {
        x = _x; y = _y;

        geo[0] = x;
        geo[1] = y;
        geo[2] = width;
        geo[3] = height;
    }
    
    void setSize(float _width, float _height)
    {
        width = _width; height = _height;

        geo[0] = x;
        geo[1] = y;
        geo[2] = width;
        geo[3] = height;
    }
    
protected:
    int tris;
    float x, y, width, height;
};


#define PORT 7000

CircularBuffer<SGMessage> g_msgBuffer(50);
GLuint g_program = 0;
std::map<std::string, SGObject *> g_objects;


class ExamplePacketListener : public osc::OscPacketListener
{
protected:
        
    std::string GetId( const osc::ReceivedMessage& m )
    {
        std::string theId;
        std::ostringstream ss;
        
        const osc::ReceivedMessageArgument &arg = *m.ArgumentsBegin();
        if(arg.IsString())
        {
            theId = arg.AsString();
        }
        else if(arg.IsInt32())
        {
            ss << arg.AsInt32();
            theId = ss.str();
        }
        else if(arg.IsFloat())
        {
            ss << arg.AsFloat();
            theId = ss.str();
        }
        else
        {
            fprintf(stderr, "ExamplePacketListener::GetId: unable to get id");
        }
        
        return theId;
    }
    
    virtual void ProcessMessage( const osc::ReceivedMessage& m, 
                                 const IpEndpointName& remoteEndpoint )
    {
        try
        {
            SGMessage msg;
            msg.objectId = GetId(m);
            osc::ReceivedMessageArgumentIterator i = ++m.ArgumentsBegin();
            
            if(strcmp( m.AddressPattern(), "/sg/line" ) == 0)
            {
                msg.type = SGMessage::LINE;
                msg.position.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.position.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.r = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.g = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.b = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.a = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/rect" ) == 0)
            {
                msg.type = SGMessage::RECT;
                msg.position.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.position.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.r = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.g = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.b = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.a = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/ellipse" ) == 0)
            {
                msg.type = SGMessage::ELLIPSE;
                msg.position.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.position.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.r = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.g = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.b = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.a = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/image" ) == 0)
            {
                msg.type = SGMessage::IMAGE;
                msg.str = (i++)->AsString();
                msg.position.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.position.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.r = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.g = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.b = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.a = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/remove" ) == 0)
            {
                msg.type = SGMessage::REMOVE;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/position" ) == 0)
            {
                msg.type = SGMessage::POSITION;
                msg.position.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.position.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/size" ) == 0)
            {
                msg.type = SGMessage::SIZE;
                msg.size.x = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.size.y = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/color" ) == 0)
            {
                msg.type = SGMessage::COLOR;
                msg.color.r = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.g = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.b = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                msg.color.a = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/red" ) == 0)
            {
                msg.type = SGMessage::RED;
                msg.color.r = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/green" ) == 0)
            {
                msg.type = SGMessage::GREEN;
                msg.color.g = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/blue" ) == 0)
            {
                msg.type = SGMessage::BLUE;
                msg.color.b = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
            else if(strcmp( m.AddressPattern(), "/sg/alpha" ) == 0)
            {
                msg.type = SGMessage::ALPHA;
                msg.color.a = i->IsInt32() ? i->AsInt32() : i->AsFloat(); i++;
                g_msgBuffer.put(msg);
            }
        }
        catch( osc::Exception& e )
        {
            // any parsing errors such as unexpected argument types, or 
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
                << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }
};

ExamplePacketListener listener;
UdpListeningReceiveSocket receiver(IpEndpointName( IpEndpointName::ANY_ADDRESS, PORT ), &listener );

// Edgar:  We need a function like this for the thread to run at its creation time.
// Arguments could be passed via (void *)ptr -- in this case the argument is a 
// string that is printed to indicate that the thread has started successfully.
void *pthread_start_function( void *ptr )
{
     receiver.RunUntilSigInt();
     return NULL;
}


/******************************************************************************
 Now back to the OpenGLES code ...   First we have Defines
******************************************************************************/

// Width and height of the window
int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;
// #define WINDOW_WIDTH 480
// #define WINDOW_HEIGHT    320

/*!****************************************************************************
 @Function      TestEGLError
 @Input         pszLocation     location in the program where the error took
                                place. ie: function name
 @Return        bool            true if no EGL error was detected
 @Description   Tests for an EGL error and prints it
******************************************************************************/
bool TestEGLError(const char* pszLocation)
{
    /*
        eglGetError returns the last error that has happened using egl,
        not the status of the last called function. The user has to
        check after every single egl call or at least once every frame.
    */
    EGLint iErr = eglGetError();
    if (iErr != EGL_SUCCESS)
    {
        printf("%s failed (%d).\n", pszLocation, iErr);
        return false;
    }

    return true;
}

void normalBlend()
{
    glEnable(GL_BLEND);
    // normal blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void additiveBlend()
{
    glEnable(GL_BLEND);
    // additive blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}


/*!****************************************************************************
 @Function      main
 @Input         argc        Number of arguments
 @Input         argv        Command line arguments
 @Return        int         result code to OS
 @Description   Main function of the program
******************************************************************************/
int main(int argc, char **argv)
{
    if(argc == 3)
    {
        WINDOW_WIDTH = atoi(argv[1]);
        WINDOW_HEIGHT = atoi(argv[2]);
    }
    
    pthread_t threadHandlesOSC;
    pthread_create( &threadHandlesOSC, NULL, pthread_start_function, NULL);
    
    SGMessage msg;
    SGObject::SCREEN_WIDTH = WINDOW_WIDTH;
    SGObject::SCREEN_HEIGHT = WINDOW_HEIGHT;
    
    
    // EGL variables
    EGLDisplay          eglDisplay  = 0;
    EGLConfig           eglConfig   = 0;
    EGLSurface          eglSurface  = 0;
    EGLContext          eglContext  = 0;
    GLuint  ui32Vbo = 0; // Vertex buffer object handle
    
    EGLNativeWindowType wndType = (EGLNativeWindowType) NULL;
    
#ifdef RASPBERRY_PI
    
    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    uint32_t display_width;
    uint32_t display_height;
   
    int success;
#endif // RASPBERRY_PI

    /*
        EGL has to create a context for OpenGL ES. Our OpenGL ES resources
        like textures will only be valid inside this context
        (or shared contexts).
        Creation of this context takes place at step 7.
    */
    EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    
    // STShaderProgram * shPg = new STShaderProgram();
    // shPg->LoadVertexShader("rect.vsh");
    // shPg->LoadFragmentShader("rect.fsh");
    
        
    float far = 100;
    float near = 0;

    // Matrix used for projection model view (PMVMatrix)
    // orthographic projection
    
    float aspect = ((float)WINDOW_WIDTH)/((float) WINDOW_HEIGHT);
    float pfIdentity[] =
    {
        2.0/aspect,0.0f,0.0f,0.0f,
        0.0f,2.0f/1.0,0.0f,0.0f,
        0.0f,0.0f,1.0f/(far-near),-near/(far-near),
        0.0f,0.0f,0.0f,1.0f
    };

    // Fragment and vertex shaders code
    const char* pszFragShader;
    std::ifstream inFrag("rect.fsh");
    std::stringstream ssFrag;
    ssFrag << inFrag.rdbuf();
    std::string strFrag = ssFrag.str();
    pszFragShader = strFrag.c_str();


    const char* pszVertShader;
    std::ifstream inVert("rect.vsh");
    std::stringstream ssVert;
    ssVert << inVert.rdbuf();
    std::string strVert = ssVert.str();
    pszVertShader = strVert.c_str();
    

    /*
        Step 1 - Get the default display.
        EGL uses the concept of a "display" which in most environments
        corresponds to a single physical screen. Since we usually want
        to draw to the main screen or only have a single screen to begin
        with, we let EGL pick the default display.
        Querying other displays is platform specific.
    */
    eglDisplay = eglGetDisplay((EGLNativeDisplayType)0);

    /*
        Step 2 - Initialize EGL.
        EGL has to be initialized with the display obtained in the
        previous step. We cannot use other EGL functions except
        eglGetDisplay and eglGetError before eglInitialize has been
        called.
        If we're not interested in the EGL version number we can just
        pass NULL for the second and third parameters.
    */
    EGLint iMajorVersion, iMinorVersion;
    if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
    {
        printf("Error: eglInitialize() failed.\n");
        goto cleanup;
    }

    /*
        Step 3 - Make OpenGL ES the current API.
        EGL provides ways to set up OpenGL ES and OpenVG contexts
        (and possibly other graphics APIs in the future), so we need
        to specify the "current API".
    */
    eglBindAPI(EGL_OPENGL_ES_API);

    if (!TestEGLError("eglBindAPI"))
    {
        goto cleanup;
    }

    /*
        Step 4 - Specify the required configuration attributes.
        An EGL "configuration" describes the pixel format and type of
        surfaces that can be used for drawing.
        For now we just want to use a 16 bit RGB surface that is a
        Window surface, i.e. it will be visible on screen. The list
        has to contain key/value pairs, terminated with EGL_NONE.
     */
    EGLint pi32ConfigAttribs[5];
    pi32ConfigAttribs[0] = EGL_SURFACE_TYPE;
    pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
    pi32ConfigAttribs[2] = EGL_RENDERABLE_TYPE;
    pi32ConfigAttribs[3] = EGL_OPENGL_ES2_BIT;
    pi32ConfigAttribs[4] = EGL_NONE;

    /*
        Step 5 - Find a config that matches all requirements.
        eglChooseConfig provides a list of all available configurations
        that meet or exceed the requirements given as the second
        argument. In most cases we just want the first config that meets
        all criteria, so we can limit the number of configs returned to 1.
    */
    int iConfigs;
    if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
    {
        printf("Error: eglChooseConfig() failed.\n");
        goto cleanup;
    }

    /*
        Step 6 - Create a surface to draw to.
        Use the config picked in the previous step and the native window
        handle when available to create a window surface. A window surface
        is one that will be visible on screen inside the native display (or
        fullscreen if there is no windowing system).
        Pixmaps and pbuffers are surfaces which only exist in off-screen
        memory.
    */
    
#ifdef RASPBERRY_PI
    
    bcm_host_init();
    
    // create an EGL window surface, passing context width/height
    success = graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
    if ( success < 0 )
    {
        fprintf(stderr, "SimpleGraphics: error in graphics_get_display_size\n");
        return EGL_FALSE;
    }
   

    // You can hardcode the resolution here:
    // display_width = WINDOW_WIDTH;
    // display_height = WINDOW_HEIGHT;

    SGObject::SCREEN_WIDTH = display_width;
    SGObject::SCREEN_HEIGHT = display_height;


   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = display_width;
   dst_rect.height = display_height;

   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = display_width << 16;
   src_rect.height = display_height << 16;   

   dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   dispman_update = vc_dispmanx_update_start( 0 );
   
   dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
                           20/*layer*/, &dst_rect, 0/*src*/,
                           &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);
   //dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
   //                          0/*layer*/, &dst_rect, 0/*src*/,
   //                      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
      
   nativewindow.element = dispman_element;
   nativewindow.width = display_width;
   nativewindow.height = display_height;
   vc_dispmanx_update_submit_sync( dispman_update );
#endif // RASPBERRY_PI

    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, wndType, NULL);

    if (!TestEGLError("eglCreateWindowSurface"))
    {
        goto cleanup;
    }

    /*
        Step 7 - Create a context.
    */
    eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, ai32ContextAttribs);
    if (!TestEGLError("eglCreateContext"))
    {
        goto cleanup;
    }

    /*
        Step 8 - Bind the context to the current thread and use our
        window surface for drawing and reading.
        Contexts are bound to a thread. This means you don't have to
        worry about other threads and processes interfering with your
        OpenGL ES application.
        We need to specify a surface that will be the target of all
        subsequent drawing operations, and one that will be the source
        of read operations. They can be the same surface.
    */
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    if (!TestEGLError("eglMakeCurrent"))
    {
        goto cleanup;
    }

    /*
        Step 9 - Draw something with OpenGL ES.
        At this point everything is initialized and we're ready to use
        OpenGL ES to draw something on the screen.
    */

    GLuint uiFragShader, uiVertShader;      // Used to hold the fragment and vertex shader handles
    GLuint uiProgramObject;             // Used to hold the program handle (made out of the two previous shaders

    // Create the fragment shader object
    uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Load the source code into it
    glShaderSource(uiFragShader, 1, (const char**)&pszFragShader, NULL);

    // Compile the source code
    glCompileShader(uiFragShader);

    // Check if compilation succeeded
    GLint bShaderCompiled;
    glGetShaderiv(uiFragShader, GL_COMPILE_STATUS, &bShaderCompiled);

    if (!bShaderCompiled)
    {
        // An error happened, first retrieve the length of the log message
        int i32InfoLogLength, i32CharsWritten;
        glGetShaderiv(uiFragShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

        // Allocate enough space for the message and retrieve it
        char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(uiFragShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);

        // Displays the error
        printf("Failed to compile fragment shader: %s\n", pszInfoLog);
        delete [] pszInfoLog;
        goto cleanup;
    }

    // Loads the vertex shader in the same way
    uiVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(uiVertShader, 1, (const char**)&pszVertShader, NULL);
    glCompileShader(uiVertShader);
    glGetShaderiv(uiVertShader, GL_COMPILE_STATUS, &bShaderCompiled);

    if (!bShaderCompiled)
    {
        int i32InfoLogLength, i32CharsWritten;
        glGetShaderiv(uiVertShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
        char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(uiVertShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
        printf("Failed to compile vertex shader: %s\n", pszInfoLog);
        delete [] pszInfoLog;
        goto cleanup;
    }

    // Create the shader program
    uiProgramObject = glCreateProgram();

    // Attach the fragment and vertex shaders to it
    glAttachShader(uiProgramObject, uiFragShader);
    glAttachShader(uiProgramObject, uiVertShader);

    // Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
    glBindAttribLocation(uiProgramObject, VERTEX_ARRAY, "myVertex");

    // Link the program
    glLinkProgram(uiProgramObject);

    // Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(uiProgramObject, GL_LINK_STATUS, &bLinked);

    if (!bLinked)
    {
        int ui32InfoLogLength, ui32CharsWritten;
        glGetProgramiv(uiProgramObject, GL_INFO_LOG_LENGTH, &ui32InfoLogLength);
        char* pszInfoLog = new char[ui32InfoLogLength];
        glGetProgramInfoLog(uiProgramObject, ui32InfoLogLength, &ui32CharsWritten, pszInfoLog);
        printf("Failed to link program: %s\n", pszInfoLog);
        delete [] pszInfoLog;
        goto cleanup;
    }

    // Actually use the created program
    glUseProgram(uiProgramObject);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black background

    normalBlend();
    
    g_program = uiProgramObject;
    
    // **** Here we run the main graphics loop for controlling the GFX processor.  This loop
    // loop runs indefinitely until the user types Cntrl-C (or "killall" command) to stop
    // the process. ****
    while (1)
    {
        while(g_msgBuffer.get(msg))
        {
            switch(msg.type)
            {
                case SGMessage::RECT:
                {
                    SGObject * r = NULL;
                    if(g_objects.count(msg.objectId))
                        r = g_objects[msg.objectId];
                    else
                    {
                        r = new SGRectangle(msg.position.x, msg.position.y,
                            msg.size.x, msg.size.y);
                        r->setShaderProgram(uiProgramObject);
                        g_objects[msg.objectId] = r;
                    }
                    
                    r->processMessage(msg);
                }
                
                break;
                
                case SGMessage::IMAGE:
                {
                    SGObject * i = NULL;
                    if(g_objects.count(msg.objectId))
                        i = g_objects[msg.objectId];
                    else
                    {
                        i = new SGImage(msg.str, msg.position.x, msg.position.y,
                            msg.size.x, msg.size.y);
                        i->setShaderProgram(uiProgramObject);
                        g_objects[msg.objectId] = i;
                    }
                    
                    i->processMessage(msg);
                }
                
                break;
                
                case SGMessage::LINE:
                {
                    SGObject * e = NULL;
                    if(g_objects.count(msg.objectId))
                        e = g_objects[msg.objectId];
                    else
                    {
                        e = new SGLine(msg.position.x, msg.position.y,
                            msg.size.x, msg.size.y);
                        e->setShaderProgram(uiProgramObject);
                        g_objects[msg.objectId] = e;
                    }
                    
                    e->processMessage(msg);
                }
                break;
                
                case SGMessage::ELLIPSE:
                {
                    SGObject * e = NULL;
                    if(g_objects.count(msg.objectId))
                        e = g_objects[msg.objectId];
                    else
                    {
                        e = new SGEllipse(msg.position.x, msg.position.y,
                            msg.size.x, msg.size.y);
                        e->setShaderProgram(uiProgramObject);
                        g_objects[msg.objectId] = e;
                    }
                    
                    e->processMessage(msg);
                }
                break;
                
                case SGMessage::REMOVE:
                {
                    if(g_objects.count(msg.objectId))
                    {
                        SGObject * o = g_objects[msg.objectId];
                        delete o;
                        g_objects.erase(msg.objectId);
                    }
                }
                break;
                
                default:
                    if(g_objects.count(msg.objectId))
                    {
                        SGObject * o = g_objects[msg.objectId];
                        o->processMessage(msg);
                    }
                break;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
        // First gets the location of that variable in the shader using its name
        int i32Location = glGetUniformLocation(uiProgramObject, "myPMVMatrix");
        
        // Then passes the matrix to that variable
        glUniformMatrix4fv( i32Location, 1, GL_FALSE, pfIdentity);
        
        for(std::map<std::string,SGObject*>::iterator i = g_objects.begin();
            i != g_objects.end(); i++)
        {
            i->second->render();
        }
        
        /*
          Swap Buffers.
          Brings to the native display the current render surface.
        */
        eglSwapBuffers(eglDisplay, eglSurface);
        
        //usleep((1000000/30)-10000);
        usleep((1000000/60));
    }

    // Frees the OpenGL handles for the program and the 2 shaders
    glDeleteProgram(uiProgramObject);
    glDeleteShader(uiFragShader);
    glDeleteShader(uiVertShader);

    /*
        Step 10 - Terminate OpenGL ES and destroy the window (if present).
        eglTerminate takes care of destroying any context or surface created
        with this display, so we don't need to call eglDestroySurface or
        eglDestroyContext here.
    */
cleanup:
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(eglDisplay);
    return 0;
}

/******************************************************************************
 End of file (OGLES2HelloTriangle_NullWS.cpp)
******************************************************************************/

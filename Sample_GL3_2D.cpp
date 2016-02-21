#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

//725-726
//848-856
using namespace std;
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode);
void draw3DObject (struct VAO* vao);
float x_min=0,x_max=1200,y_min=600,y_max=0;
float camera_rotation_angle=90;
double mouseX,mouseY,h,theta=M_PI,zoom_f=1,state_mx=0,state_my=0;
double friction=0.1;
int state=0;
int line_vis=1,state_m=0,state_k=1;
int score=0;
int flag[10],flagc[10];
int lives=3;
int state_q=0;
GLfloat air_friction=0.004;
struct VAO{
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};

typedef struct VAO VAO;
VAO* line;
struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;
glm::mat4 VP;
glm::mat4 MVP;	
GLuint programID;
void trans_rot(VAO* object,GLfloat xx,GLfloat yy,GLfloat a){
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle = glm::translate (glm::vec3(xx,yy, 0));
    glm::mat4 rotateRectangle = glm::rotate((float)(a*M_PI/180.0f), glm::vec3(0,0,1)); 
    Matrices.model *= (translateRectangle*rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(object);
}
void rotate_a(VAO* object,GLfloat a,GLfloat x,GLfloat y){
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle = glm::translate (glm::vec3(-x,-y, 0));
    glm::mat4 rotateRectangle = glm::rotate((float)(a*M_PI/180.0f), glm::vec3(0,0,1)); 
    glm::mat4 translateRectangle1 = glm::translate (glm::vec3(x,y, 0));
    Matrices.model *= (translateRectangle1*rotateRectangle*translateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(object);
}
class Circle{
    public:
        GLfloat x,y,radius,in_hspeed,in_vspeed,h_speed,v_speed,angle,mass;
        VAO* object;
        void init(GLfloat xx,GLfloat yy,GLfloat r,GLfloat rr,GLfloat g,GLfloat b){
            in_hspeed=in_vspeed=h_speed=v_speed=0.0;
            x=xx,y=yy;
            mass=M_PI*r*r;
            radius=r;
            object=createCircle(0.0,0.0,0.0,radius,54,rr,g,b);
        }
        void transform(GLfloat xx,GLfloat yy,GLfloat a){x=xx,y=yy,angle=a,trans_rot(object,x,y,a);}
    private:
        VAO* createCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,double r, double g, double bb)
        {
            int numberOfVertices = numberOfSides + 2,i;

            GLfloat twicePi = 2.0f * M_PI;

            GLfloat circleVerticesX[numberOfVertices];
            GLfloat circleVerticesY[numberOfVertices];
            GLfloat circleVerticesZ[numberOfVertices];

            circleVerticesX[0] = x;
            circleVerticesY[0] = y;
            circleVerticesZ[0] = z;

            for ( i = 1; i < numberOfVertices; i++ ) {
                circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
                circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
                circleVerticesZ[i] = z;
            }

            GLfloat allCircleVertices[( numberOfVertices ) * 3];
            GLfloat allCircleColors[( numberOfVertices ) * 3 * 3];

            for ( int i = 0; i < numberOfVertices; i++ )
            {
                allCircleVertices[i * 3] = circleVerticesX[i];
                allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
                allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
            }
            for (int j=0;j<(numberOfVertices*3);j++){
                if(j%3==0)allCircleColors[j]=r;
                else if(j%3==1)allCircleColors[j]=g;
                else allCircleColors[j]=bb;
            }
            VAO* ret=create3DObject(GL_TRIANGLE_FAN,numberOfVertices, allCircleVertices, allCircleColors, GL_TRIANGLES);
            return ret;
        }
};
class Rectangle{
    public:
        GLfloat x,y,cx,cy,width,height,angle,elasticity,mass,h_speed,v_speed;
        bool rotate;
        void init(GLfloat xx,GLfloat yy,GLfloat w,GLfloat h,GLfloat e,bool rot,GLfloat r,GLfloat g,GLfloat b,bool f=0){
            GLfloat vertex_buffer_data[18];
            GLfloat color_buffer_data[18];
            rotate=rot;
            elasticity=e;
            angle=0;
            x=xx,y=yy;
            width=w;
            height=h;
            mass=width*height;
            cx=x-width/2,cy=y-height/2;
            genRect(0.0f,0.0f,0.0f,-height,-width,-height,-width,0.0f,vertex_buffer_data);
            genColor(r/255.0,g/255.0,b/255.0,color_buffer_data);

            if(!f)object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
            else object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINES);
        }
        void transform(GLfloat xx,GLfloat yy,GLfloat a,int flag=0){
            if(!flag)x=xx,y=yy,angle=a,cx=x-width/2,cy=y-height/2;
            trans_rot(object,xx,yy,a);
        }
    private:
        VAO* object;
        void genRect(GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2,GLfloat x3,GLfloat y3,GLfloat x4,GLfloat y4,GLfloat vertex_buffer_data[]){
            vertex_buffer_data[0]=x1;
            vertex_buffer_data[1]=y1;
            vertex_buffer_data[2]=0;

            vertex_buffer_data[3]=x2;
            vertex_buffer_data[4]=y2;
            vertex_buffer_data[5]=0;

            vertex_buffer_data[6]=x3;
            vertex_buffer_data[7]=y3;
            vertex_buffer_data[8]=0;

            vertex_buffer_data[9]=x3;
            vertex_buffer_data[10]=y3;
            vertex_buffer_data[11]=0;

            vertex_buffer_data[12]=x4;
            vertex_buffer_data[13]=y4;
            vertex_buffer_data[14]=0;

            vertex_buffer_data[15]=x1;
            vertex_buffer_data[16]=y1;
            vertex_buffer_data[17]=0;
        }
        void genColor(GLfloat x1,GLfloat y1,GLfloat z1,GLfloat vertex_buffer_data[]){
            vertex_buffer_data[0]=x1;
            vertex_buffer_data[1]=y1;
            vertex_buffer_data[2]=z1;

            vertex_buffer_data[3]=x1;
            vertex_buffer_data[4]=y1;
            vertex_buffer_data[5]=z1;

            vertex_buffer_data[6]=x1;
            vertex_buffer_data[7]=y1;
            vertex_buffer_data[8]=z1;

            vertex_buffer_data[9]=x1;
            vertex_buffer_data[10]=y1;
            vertex_buffer_data[11]=z1;

            vertex_buffer_data[12]=x1;
            vertex_buffer_data[13]=y1;
            vertex_buffer_data[14]=z1;

            vertex_buffer_data[15]=x1;
            vertex_buffer_data[16]=y1;
            vertex_buffer_data[17]=z1;
        }
};
vector<Rectangle> obstacles;
vector<Rectangle> bcd;
vector<Rectangle> bcd1;
vector<vector<int> > segments(10);
vector<Rectangle> bar;
VAO *life_bor1,*life_bor2,*life_bor3,*life_bor4;
Rectangle life_bar,life_bar1,life_bar2;
class Pig: public Circle{
    public:
        bool dead;
        void transform(GLfloat xx,GLfloat yy,GLfloat a){
            if(dead!=1){
                x=xx,y=yy,angle=a;
                trans_rot(object,x,y,a);
            }
        }
};
vector<Pig> pigs;
class Ball: public Circle{
    public:
        void update(){
            transform(x,y,0);
            x+=h_speed,y-=v_speed;
            for(int i=0;i<obstacles.size();i++){
                if(collide(obstacles[i])){
                    if(i>0 && !flag[i])score+=2,flag[i]=1;
                    if(x>obstacles[i].cx-obstacles[i].width/2 && x<obstacles[i].cx+obstacles[i].width/2 && (obstacles[i].cy-y-radius)>=0 && (obstacles[i].cy-y-radius)<=(obstacles[i].height/2)){
                        y=obstacles[i].cy-obstacles[i].height/2-radius;
                        v_speed=in_vspeed*obstacles[i].elasticity;
                        h_speed=in_hspeed*obstacles[i].elasticity;
                        in_vspeed=v_speed;
                        in_hspeed=h_speed;
                    }
                    else if(x>obstacles[i].cx-obstacles[i].width/2 && x<obstacles[i].cx+obstacles[i].width/2 && (obstacles[i].cy-y+radius)<=0 && (y-radius-obstacles[i].cy)<=(obstacles[i].height/2)){
                        y=obstacles[i].cy+obstacles[i].height/2+radius;
                        v_speed=-in_vspeed*obstacles[i].elasticity;
                        h_speed=in_hspeed*obstacles[i].elasticity;
                        in_vspeed=v_speed;
                        in_hspeed=h_speed;
                    }
                    else if(x<=obstacles[i].cx-obstacles[i].width/2){
                        x=obstacles[i].cx-obstacles[i].width/2-radius;
                        if(i<4)obstacles[i].h_speed=((2*obstacles[i].mass)/(mass+obstacles[i].mass))*h_speed;
                        if(obstacles[i].rotate)after_collide_rotate(i,0);
                        else{
                            if(i<4)obstacles[i].h_speed=((2*obstacles[i].mass)/(mass+obstacles[i].mass))*h_speed;
                            v_speed=v_speed*obstacles[i].elasticity;
                            h_speed=-in_hspeed*obstacles[i].elasticity;
                            in_vspeed=v_speed;
                            in_hspeed=h_speed;
                        }
                    }
                    else{
                        x=obstacles[i].cx+obstacles[i].width/2+radius;
                        if(i<4)obstacles[i].h_speed=((2*obstacles[i].mass)/(mass+obstacles[i].mass))*h_speed;
                        if(obstacles[i].rotate)after_collide_rotate(i,1);
                        else{
                            if(i<4)obstacles[i].h_speed=((2*obstacles[i].mass)/(mass+obstacles[i].mass))*h_speed;
                            v_speed=v_speed*obstacles[i].elasticity;
                            h_speed=-in_hspeed*obstacles[i].elasticity;
                            in_vspeed=v_speed;
                            in_hspeed=h_speed;
                        }
                    }
                    break;
                }
            }
            for(int i=0;i<pigs.size();i++){
                if(collide_circle(pigs[i]) && !flagc[i])pigs[i].dead=1,score+=3,flagc[i]=1;
            }
        }
    private:
        void transform(GLfloat xx,GLfloat yy,GLfloat a){
            x=xx,y=yy,angle=a,trans_rot(object,x,y,a);
        }
        void after_collide_shift(int i){
            h_speed=((mass-obstacles[i].mass)/(mass+obstacles[i].mass))*h_speed;
            in_hspeed=h_speed;
            obstacles[i].h_speed=((2*obstacles[i].mass)/(mass+obstacles[i].mass))*h_speed;
        }
        void after_collide_rotate(int i,int f){
            h_speed=-in_hspeed*obstacles[i].elasticity;
            v_speed=in_vspeed*obstacles[i].elasticity;
            in_hspeed=h_speed;
            in_vspeed=v_speed;
            if(!f)obstacles[i].angle+=3;
            else obstacles[i].angle-=3;
        }
        bool collide(Rectangle r){
            double circleDistanceX = abs(x - r.cx);
            double circleDistanceY = abs(y - r.cy);

            if (circleDistanceX > (r.width/2 + radius)) { return false; }
            if (circleDistanceY > (r.height/2 + radius)) { return false; }

            if (circleDistanceX <= (r.width/2)) { return true; } 
            if (circleDistanceY <= (r.height/2)) { return true; }

            double cornerDistance_sq = pow((circleDistanceX - r.width/2),2) +
                pow((circleDistanceY - r.height/2),2);

            return (cornerDistance_sq <= pow(radius,2));

        }
        bool collide_circle(Pig p){
            double distanceX=p.x-x;
            double distanceY=p.y-y;
            double magnitude=sqrt(distanceX*distanceX + distanceY*distanceY);
            return magnitude < radius+p.radius;
        }
};
Ball ball;
double getAngle() {
    double angle = (double)atan2(mouseY - ball.y, mouseX - ball.x);
    angle*=180/M_PI;

    if(angle < 0){
        angle += 360;
    }

    return angle*M_PI/180;
}
void update_obs(){
    for(int i=0;i<obstacles.size();i++){
        obstacles[i].x+=obstacles[i].h_speed,obstacles[i].y+=obstacles[i].v_speed;
        obstacles[i].h_speed=0;
        obstacles[i].transform(obstacles[i].x,obstacles[i].y,min(fabs((double)obstacles[i].angle),90.0));
        if(obstacles[i].rotate && obstacles[i].angle>0 && obstacles[i].angle<90)obstacles[i].angle+=3;
        if(obstacles[i].rotate && obstacles[i].angle<0 && obstacles[i].angle>-90)obstacles[i].angle-=3;
        if(i==1 && obstacles[1].angle>=27.0 && !flagc[1])pigs[1].dead=1,score+=3,flagc[1]=1;
        if(obstacles[i].angle>=90){
            obstacles[i].init(obstacles[i].x+obstacles[i].height,obstacles[i].y,obstacles[i].height,obstacles[i].width,0.5,0,0.0,0.0,0.0);
        }
        else if(obstacles[i].angle==-93){
            GLfloat t=obstacles[i].height;
            obstacles[i].height=obstacles[i].width;
            obstacles[i].width=t;
            obstacles[i].rotate=0;
            obstacles[i].angle=0;
        }
    }
}
void update_pigs(){
    for(int i=0;i<pigs.size();i++){
        if(!pigs[i].dead){
            if(i==3 && obstacles[i].h_speed)pigs[i].x+=obstacles[i].h_speed;
            pigs[i].transform(pigs[i].x,pigs[i].y,pigs[i].angle);
        }
    }
}
void update_bcd(){
    int temp=score;
    if(score==0){
        int tscore=0;
        for(int i=0;i<segments[tscore].size();i++){
            int temp=segments[tscore][i];
            if(score>=10)bcd[temp].transform(bcd[temp].x-100,bcd[temp].y,bcd[temp].angle,1);
            else bcd[temp].transform(bcd[temp].x,bcd[temp].y,bcd[temp].angle,1);
        }
    }
    while(score!=0){
        int tscore=score%10;
        for(int i=0;i<segments[tscore].size();i++){
            int temp=segments[tscore][i];
            if(score>=10)bcd1[temp].transform(bcd1[temp].x+65,bcd1[temp].y,bcd1[temp].angle,1);
            else bcd[temp].transform(bcd[temp].x,bcd[temp].y,bcd[temp].angle,1);
        }
        score/=10;
    }
    score=temp;
}
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    //printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_A:
                theta+=(10*M_PI/180);
                break;
            case GLFW_KEY_D:
                theta-=(10*M_PI/180);
                break;
            case GLFW_KEY_F:
                h+=10;
                break;
            case GLFW_KEY_S:
                h-=10;
                break;
            case GLFW_KEY_SPACE:
                if(state==0)state=1;
                else state*=-1;
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_Q:
                state=1;
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    double a,b;
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE && state==1){
                state*=-1;
                a=ball.y-mouseY;a=-a;
                b=ball.x-mouseX;b=-b;
                h=hypot(a,b);
                theta=atan2(a,b);
                theta=M_PI-theta;
                /* theta*=-1; */
                line_vis=0;
            }
            if(action == GLFW_PRESS){
                if(sqrt(pow(mouseY-ball.y,2)+pow(mouseX-ball.x,2))<=10){
                    state=1;
                }
                else
                    ball.init(150.0,390.0,10.0,255.0,0.0,0.0),state=0,theta=180,line_vis=1;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if(action==GLFW_PRESS){
                state_m=1;
                state_mx=mouseX,state_my=mouseY;
            }
            if(action==GLFW_RELEASE)
                state_m=0;
            break;
        default:
            break;
    }
}
/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //Matrices.projection = glm::ortho(0.0f, 2400.0f, -1200.0f, 0.0f, 0.1f, 500.0f);
    Matrices.projection = glm::ortho(x_min, x_max, y_min, y_max, 0.1f, 500.0f);
}

// Creates the triangle object used in this sample code
/*void createTriangle ()
{
     ONLY vertices between the bounds specified in glm::ortho will be visible on screen 

     Define vertex array as used in glBegin (GL_TRIANGLES) 
    static const GLfloat vertex_buffer_data [] = {
        ball.x,ball.y,0, // vertex 0
        -1,-1,0, // vertex 1
        1,-1,0, // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 0
        0,1,0, // color 1
        0,0,1, // color 2 };

    // create3DObject creates and returns a handle to a VAO that can be used later
    triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}*/
/*void createRectangle ()
  {
// GL3 accepts only Triangles. Quads are not supported
static const GLfloat vertex_buffer_data [] = {
1,-1,0, rtex 1
1,-2,0, // vertex 2
2, -2,0, // vertex 3

2, -2,0, // vertex 3
2, -1,0, // vertex 4
1,-1,0  // vertex 1
};

static const GLfloat color_buffer_data [] = {
1,0,0, // color 1
00,1, // color 2
0,1,0, // color 3

0,1,0, // color 3
0.3,0.3,0.3, // color 4
1,0,0  // color 1
};
rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}*/
// Creates the rectangle object used in this sample code

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
    /*BoilerPlate Code*/
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (programID);
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    glm::vec3 target (0, 0, 0);
    glm::vec3 up (0, 1, 0);
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    VP = Matrices.projection * Matrices.view;
    
    if(line_vis){
        trans_rot(line,ball.x,ball.y,-theta*180/M_PI);
    }
    trans_rot(life_bor1,10.0,40.0,0);
    trans_rot(life_bor2,10.0,70.0,0);
    trans_rot(life_bor4,308.0,40.0,0);
    trans_rot(life_bor3,10.0,40.0,0);
    if(lives>=1)life_bar.transform(life_bar.x,life_bar.y,life_bar.angle);
    if(lives>=2)life_bar1.transform(life_bar1.x,life_bar1.y,life_bar1.angle);
    if(lives>=3)life_bar2.transform(life_bar2.x,life_bar2.y,life_bar2.angle);
    ball.update();    
    update_pigs();
    update_obs();
    update_bcd();
    
    for(int i=0;i<ceil(h);i++)
        bar[i].transform(bar[i].x,bar[i].y,bar[i].angle);
}
/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    //glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    //createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
     GLfloat vertex_buffer_data [] = {
         0,0,0,
         50,0,0
    };

    GLfloat color_buffer_data [] = {
        0.0f,0.0f,0.0f, // color 0
        0.0f,0.0f,0.0f,
    };
    line = create3DObject(GL_LINES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
    GLfloat vertex_buffer_data1 [] = {
         0,0,0,
         0,30,0
    };
    life_bor1 = create3DObject(GL_LINES, 3, vertex_buffer_data1, color_buffer_data, GL_FILL);
    GLfloat vertex_buffer_data2 [] = {
         0,0,0,
         298,0,0
    };
    life_bor2 = create3DObject(GL_LINES, 3, vertex_buffer_data2, color_buffer_data, GL_FILL);
    GLfloat vertex_buffer_data3 [] = {
         0,0,0,
         298,0,0
    };
    life_bor3 = create3DObject(GL_LINES, 3, vertex_buffer_data3, color_buffer_data, GL_FILL);
    GLfloat vertex_buffer_data4 [] = {
         0,0,0,
         0,30,0
    };
    life_bor4 = create3DObject(GL_LINES, 3, vertex_buffer_data4, color_buffer_data, GL_FILL);
    life_bar.init(108.0,65.0,96.0,20.0,0.0,0.0,0.0,153.0,0.0);
    life_bar1.init(206.0,65.0,96.0,20.0,0.0,0.0,0.0,153.0,0.0);
    life_bar2.init(304.0,65.0,96.0,20.0,0.0,0.0,0.0,153.0,0.0);
    ball.init(150.0,390.0,10.0,255.0,0.0,0.0);
    for(int i=0;i<8;i++){
        Rectangle r;
        obstacles.push_back(r);
        switch(i){
            case 0:
                obstacles[i].init(1200.0,600.0,1200.0,200.0,0.4,0.0,153.0,76.0,0.0);
                break;
            case 1:
                obstacles[i].init(500.0,400.0,30.0,95.0,0.5,1,0.0,0.0,0.0);
                break;
            case 2:
                obstacles[i].init(800.0,400.0,60.0,45.0,0.5,0.0,0.0,0.0,0.0);
                break;
            case 3:
                obstacles[i].init(1000.0,400.0,30.0,95.0,0.5,1,0.0,0.0,0.0);
                break;
            case 4:
                obstacles[i].init(785.0,150.0,200.0,30.0,0.5,0,0.0,0.0,0.0);
                break;
            case 5:
                obstacles[i].init(700.0,150.0,30.0,150.0,0.5,0,0.0,0.0,0.0);
                break;
            case 6:
                obstacles[i].init(615.0,150.0,30.0,55.0,0.5,0,0.0,0.0,0.0);
                break;
            case 7:
                obstacles[i].init(785.0,150.0,30.0,55.0,0.5,0,0.0,0.0,0.0);
                break;
        }
    }
    for(int i=0;i<610;i++){
        Rectangle b;
        bar.push_back(b);
        if(i==0)bar[i].init(2.0,20.0,2.0,10.0,0.0,0.0,0.0,204.0,0.0);
        else if(i<100)bar[i].init(bar[i-1].x+2.0,20.0,2.0,10.0,0.0,0.0,173.0,255.0,47.0);
        else if(i>=100 && i<200)bar[i].init(bar[i-1].x+2.0,20.0,2.0,10.0,0.0,0.0,255.0,255.0,102.0);
        else if(i>=200 && i<300)bar[i].init(bar[i-1].x+2.0,20.0,2.0,10.0,0.0,0.0,255.0,255.0,0.0);
        else if(i>=300 && i<400)bar[i].init(bar[i-1].x+2.0,20.0,2.0,10.0,0.0,0.0,204.0,0.0,0.0);
        else bar[i].init(bar[i-1].x+2.0,20.0,2.0,10.0,0.0,0.0,255.0,0.0,0.0);
    }
    for(int i=0;i<4;i++){
        Pig p;
        pigs.push_back(p);
        switch(i){
            case 0:
                pigs[i].init(430.0,375.0,25.0,0.0,153.0,0.0);
                break;
            case 1:
                pigs[i].init(540.0,375.0,25.0,0.0,153.0,0.0);
                break;
            case 2:
                pigs[i].init(770.0,330.0,25.0,0.0,153.0,0.0);
                break;
            case 3:
                pigs[i].init(640.0,105.0,15.0,0.0,153.0,0.0);
                break;
        }
    }
    for(int i=0;i<7;i++){
        Rectangle r;
        bcd.push_back(r);
        bcd1.push_back(r);
        switch (i){
            case 0:
                bcd[i].init(1000.0,30.0,40.0,2.5,0.0,0.0,0.0,0.0,0.0);
                bcd1[i].init(1000.0,30.0,40.0,2.5,0.0,0.0,0.0,0.0,0.0);
                break;
            case 1:
                bcd[i].init(1000.0,72.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                bcd1[i].init(1000.0,72.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                break;
            case 2:
                bcd[i].init(1000.0,122.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                bcd1[i].init(1000.0,122.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                break;
            case 3:
                bcd[i].init(1000.0,127.0,40.0,2.5,0.0,0.0,0.0,0.0,0.0);
                bcd1[i].init(1000.0,127.0,40.0,2.5,0.0,0.0,0.0,0.0,0.0);
                break;
            case 4:
                bcd[i].init(960.0,122.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                bcd1[i].init(960.0,122.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                break;
            case 5:
                bcd[i].init(960.0,72.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                bcd1[i].init(960.0,72.0,2.5,40.0,0.0,0.0,0.0,0.0,0.0);
                break;
            case 6:
                bcd[i].init(1000.0,78.0,40.0,2.5,0.0,0.0,0.0,0.0,0.0);
                bcd1[i].init(1000.0,78.0,40.0,2.5,0.0,0.0,0.0,0.0,0.0);
                break;
            default:
                break;
        }
        for(int i=0;i<=9;i++){
            switch(i){
                case 0:
                    segments[i].push_back(0);
                    segments[i].push_back(1);
                    segments[i].push_back(2);
                    segments[i].push_back(3);
                    segments[i].push_back(4);
                    segments[i].push_back(5);
                    break;
                case 1:
                    segments[i].push_back(1);
                    segments[i].push_back(2);
                    break;
                case 2:
                    segments[i].push_back(0);
                    segments[i].push_back(1);
                    segments[i].push_back(6);
                    segments[i].push_back(3);
                    segments[i].push_back(4);
                    break;
                case 3:
                    segments[i].push_back(0);
                    segments[i].push_back(1);
                    segments[i].push_back(2);
                    segments[i].push_back(3);
                    segments[i].push_back(6);
                    break;
                case 4:
                    segments[i].push_back(1);
                    segments[i].push_back(2);
                    segments[i].push_back(6);
                    segments[i].push_back(5);
                    break;
                case 5:
                    segments[i].push_back(0);
                    segments[i].push_back(6);
                    segments[i].push_back(2);
                    segments[i].push_back(3);
                    segments[i].push_back(5);
                    break;
                case 6:
                    segments[i].push_back(0);
                    segments[i].push_back(6);
                    segments[i].push_back(2);
                    segments[i].push_back(3);
                    segments[i].push_back(4);
                    segments[i].push_back(5);
                    break;
                case 7:
                    segments[i].push_back(0);
                    segments[i].push_back(1);
                    segments[i].push_back(2);
                    break;
                case 8:
                    segments[i].push_back(0);
                    segments[i].push_back(1);
                    segments[i].push_back(2);
                    segments[i].push_back(3);
                    segments[i].push_back(4);
                    segments[i].push_back(5);
                    segments[i].push_back(6);
                    break;
                case 9:
                    segments[i].push_back(0);
                    segments[i].push_back(1);
                    segments[i].push_back(2);
                    segments[i].push_back(3);
                    segments[i].push_back(5);
                    segments[i].push_back(6);
                    break;
            }
        }
    }
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.5f, 0.8f, 0.9f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

/*    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;*/
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    if(yoffset==1){x_min/=1.02,x_max/=1.02,y_min/=1.02,y_max/=1.02,zoom_f*=1.02;}
    if(yoffset==-1){x_min*=1.02,x_max*=1.02,y_min*=1.02,y_max*=1.02,zoom_f/=1.02;}
}
int state_d=0,state_a=0,state_s=0,state_w=0,state_plus=0,state_minus=0;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
        if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)state_d=1;
        if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)state_a=1;
        if(key == GLFW_KEY_UP && action == GLFW_PRESS)state_s=1;
        if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)state_w=1;
        if(key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)state_plus=1;
        if(key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)state_minus=1;
        if(action==GLFW_RELEASE && state_d==1)state_d=0;
        if(action==GLFW_RELEASE && state_a==1)state_a=0;
        if(action==GLFW_RELEASE && state_s==1)state_s=0;
        if(action==GLFW_RELEASE && state_w==1)state_w=0;
        if(action==GLFW_RELEASE && state_plus==1)state_plus=0;
        if(action==GLFW_RELEASE && state_minus==1)state_minus=0;
        if(state_d==1)x_max+=10.0f,x_min+=10.0f;
        if(state_a==1)x_max-=10.0f,x_min-=10.0f;
        if(state_s==1)y_max+=10.0f,y_min+=10.0f;
        if(state_w==1)y_max-=10.0f,y_min-=10.0f;
        if(state_plus==1){x_min/=1.02,x_max/=1.02,y_min/=1.02,y_max/=1.02,zoom_f*=1.02;}
        if(state_minus==1){x_min*=1.02,x_max*=1.02,y_min*=1.02,y_max*=1.02,zoom_f/=1.02;}
        
        if (action == GLFW_RELEASE) {
            switch (key) {
                case GLFW_KEY_D:
                    theta+=(10*M_PI/180);
                    break;
                case GLFW_KEY_A:
                    theta-=(10*M_PI/180);
                    break;
                case GLFW_KEY_F:
                    h+=10;
                    break;
                case GLFW_KEY_S:
                    h-=10;
                    break;
                case GLFW_KEY_R:
                    ball.init(150.0,390.0,10.0,255.0,0.0,0.0),state=0,theta=180,line_vis=1,lives=3;
                    break;
                case GLFW_KEY_Q:
                    state_q=1;
                    break;
                case GLFW_KEY_SPACE:
                    state_k*=-1;
                    break;
            }
        }
        else if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_Q:
                    break;
                default:
                    break;
            }
        }
}
int main (int argc, char** argv)
{
    int width = 1200;
    int height = 600;
    GLFWwindow* window = initGLFW(width, height);
    initGL (window, width, height);
    double last_update_time = glfwGetTime(), current_time;
    flag[0]=flag[1]=flag[2]=flag[3]=0,flag[4]=0;
    flagc[0]=flagc[1]=flagc[2]=flagc[3]=0,flagc[4]=0;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        reshapeWindow(window,width,height);

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
       current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.25) { // atleast 0.5s elapsed since last frame
            if(state==-1 || state_k==-1){
                if(ball.h_speed==0 && ball.v_speed==0 && lives){
                    ball.h_speed=(h/30)*cos(theta);
                    ball.v_speed=(h/30)*sin(theta);
                    ball.in_vspeed=ball.v_speed;
                    ball.in_hspeed=ball.h_speed;
                    h=0;
                    state_k=1;
                    lives--;
                }
                else{
                    ball.v_speed-=0.981;
                    ball.v_speed-=air_friction;
                }
            }
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
        glfwGetCursorPos(window,&mouseX,&mouseY);
        if(state==1){
            double a=ball.y-mouseY;a=-a;
            double b=ball.x-mouseX;b=-b;
            h=hypot(a,b);
            theta=atan2(a,b);
            theta=M_PI-theta;
            /* theta*=-1; */
            /* cout << theta*180/M_PI << endl; */
        }
        if(state_m==1){
            x_max+=mouseX-state_mx,x_min+=mouseX-state_mx;
            y_max+=mouseY-state_my,y_min+=mouseY-state_my;
        }
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetKeyCallback(window,key_callback);
        mouseX/=zoom_f,mouseY/=zoom_f;
        if(state_q)break;
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

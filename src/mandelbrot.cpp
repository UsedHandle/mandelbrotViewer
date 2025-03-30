// https://www.shadertoy.com/view/4df3Rn
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cmath>

static int lastMouseStatePress  = GLFW_RELEASE;
static int lastPlusStatePress   = GLFW_RELEASE;
static int lastMinusStatePress  = GLFW_RELEASE; 
void pollInput(
    GLFWwindow* window,
    double& cre,
    double& cim,
    double& diam);

const char *vertexShaderSource =
    "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource =
    "#version 410 core\n"
    "out vec4 FragColor;\n"
    "uniform int fbwidth;\n"
    "uniform int fbheight;\n"
    "uniform uint niterations;\n"
    "uniform double cre;\n"
    "uniform double cim;\n"
    "uniform double diam;"
    "double minr = cre-diam*0.5, mini = cim-diam*0.5;\n"
    "double maxr = cre+diam*0.5, maxi = cim+diam*0.5;\n"
    "double stepr = (maxr-minr) / double(fbwidth);\n"
    "double stepi = (maxi-mini) / double(fbheight);\n"
    "void main()\n"
    "{\n"
    "   dvec2 c   = vec2(\n"
    "       minr+double(gl_FragCoord.x) * stepr,\n"
    "       mini+double(gl_FragCoord.y) * stepi);\n"
    "   double c2 = dot(c,c);\n"
        // skip computation inside M1 & M2
        // http://iquilezles.org/www/articles/mset_1bulb/mset1bulb.htm 
    "   if( 256.0*c2*c2 - 96.0*c2 + 32.0*c.x - 3.0 < 0.0 ){\n"
    "       FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    "       return;\n"
    "   }\n"
        // http://iquilezles.org/www/articles/mset_2bulb/mset2bulb.htm
    "   if( 16.0*(c2+2.0*c.x+1.0) - 1.0 < 0.0 ){\n"
    "       FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    "       return;\n"
    "   }\n"
    "   dvec2 z   = vec2(0.0);\n"
    "   uint iter;\n"
    "   for(iter = 0u; iter < niterations; ++iter){\n"
    "       if(dot(z,z) > 16.0f) break;" // |z| > 4 -> |z|^2 > 16
            //            ^ higher breakpoint reduces banding
    "       z = vec2(z.x*z.x - z.y*z.y,  2.0f*z.x*z.y) + c;\n"
    "   }\n" // ^ z = z^2 + c
    "   float iter1 = float(iter) - log2(log2(float(dot(z,z)))) + 4.0;\n"
    "   vec3 col1 = 0.5f + 0.5f*cos(3.0f+iter1*0.15f+vec3(0.0,0.6,1.0));\n"
    "   vec3 col = vec3(0.0f);\n"
    "	if(iter ==  niterations){\n"
    "      FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    "      return;\n"
    "   }\n"
    "   col = col1;\n"
    "   FragColor = vec4(col, 1.0f);\n"
    "}\n\0";

static int width = 720;
static int height = 720;

static int fbwidth, fbheight;

int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(width, height, "mandelbrot", NULL, NULL);
    if (window == NULL){
	fprintf(stderr, "Error: could not create window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* windowptr, int new_fbwidth, int new_fbheight){
            glViewport(0, 0, new_fbwidth, new_fbheight);
	    fbwidth  = new_fbwidth;
	    fbheight = new_fbheight; 
   });

   glfwSetWindowSizeCallback(
	window,
	[](GLFWwindow* windowptr, int new_width, int new_height){
	    width = new_width;
	    height = new_height;
    });
	

    if (!gladLoadGLLoader(
          reinterpret_cast<GLADloadproc>(glfwGetProcAddress))){
	fprintf(stderr, "Error: could not load glad\n");
        return -1;
    }

    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport(0, 0, fbwidth, fbheight);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	fprintf(stderr, "Error: Vertex Shader:\n%s\n", infoLog);
    }

	
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	fprintf(stderr, "Error: Fragment Shader:\n%s\n", infoLog);
    }

    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	fprintf(stderr, "Error: Program Linking\n%s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 
         1.0f,  1.0f, 0.0f,
        
	-1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 
         1.0f,  1.0f, 0.0f  
    }; 

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 


    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // should be GLFW_TRUE instead of true
    // but could not get to compile with it
    // (Undeclared Identifier)    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, true);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, true);    
	
    int iterations = 100.0;
    double cre = -0.5, cim = 0.0, diam = 3.0; 
    while (!glfwWindowShouldClose(window)){
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
	    pollInput(window, cre, cim, diam);
    	
	    glUniform1i(glGetUniformLocation(shaderProgram, "fbwidth"), fbwidth);
   	    glUniform1i(glGetUniformLocation(shaderProgram, "fbheight"), fbheight);
   	    glUniform1ui(glGetUniformLocation(shaderProgram, "niterations"), 
			512);
	
	    glUniform1d(glGetUniformLocation(shaderProgram, "cre"), cre);
        glUniform1d(glGetUniformLocation(shaderProgram, "cim"), cim);
        glUniform1d(glGetUniformLocation(shaderProgram, "diam"), diam);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}


void pollInput(
    GLFWwindow* window,
    double& cre,
    double& cim,
    double& diam)
{
	int mouseButtonState =
	    glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(mouseButtonState && !lastMouseStatePress){
	    double xpos, ypos;
	    glfwGetCursorPos(window, &xpos, &ypos);

	    // get offsets with origin at the center
	    // window space coordinates have the origin
	    // in the top left
	    xpos -= static_cast<double>(width)/2.0;
	    ypos -= static_cast<double>(height)/2.0;
	    ypos *= -1.0;
	   
	    // normalize coordinates
	    xpos /= static_cast<double>(width);
	    ypos /= static_cast<double>(height);
	    
	    cre += xpos*diam;
	    cim += ypos*diam;
	}
	

	// equal key is same as plus key except doesn't need
	// shift	
	int plusButtonState = glfwGetKey(window, GLFW_KEY_EQUAL);
	if(plusButtonState && !lastPlusStatePress)
		diam *= 0.75;
	

	int minusButtonState = glfwGetKey(window, GLFW_KEY_MINUS);
	if(minusButtonState && !lastMinusStatePress)
		diam *= 1.33;

	
	if(glfwGetKey(window, GLFW_KEY_0)){
		cre = -0.5;
		cim =  0.0;
		diam = 3.0;
	}	

	lastMouseStatePress = mouseButtonState;
	lastPlusStatePress  = plusButtonState;	
	lastMinusStatePress = minusButtonState;	
}

#include<gl/glut.h>
#include<GL/freeglut_ext.h>
#include<string>
#include<math.h>
#include<time.h>
#include<stdio.h>
#include<iostream>
using namespace std;
#define DISTANCE 9999 //����Ӿ�
#define random(x) (rand()%x)//����0~x-1֮��������
#define BMP_Header_Length 54  //ͼ���������ڴ���е�ƫ����
const float pi = 3.1415926f;

float WinLong = 1980 / 2 - 100;//�Ӵ�����z
float WinWide = 1080 / 2 + 200;
float camera_x, camera_y, camera_z;//����ƶ�λ��
float univer_r;//�ռ���ת

int level;//��������
int lineState;//�Ƿ�������ߣ�Ϊ-1�����ƣ�1����
int veinState;//�Ƿ��������
int projectWay;//ͶӰ��ʽ
GLuint tree, leaf;//����
//�䱾��bmp�ļ���Ҫ����ڹ�����Ŀ��
//debug���ɵ�exe��Ҫ��bmp������ͬһ�ļ��в��ܼ���

//�ж�һ�������ǲ���2����������
int power_of_two(int n)
{
	if (n <= 0) {
		return 0;
	}
	return (n & (n - 1)) == 0;
}
//��ȡbmp�ļ�������������
GLuint LoadTexture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;

	// ���ļ������ʧ�ܣ�����	
	FILE* pFile;
	if (fopen_s(&pFile, file_name, "rb")) {
		//printf_s("�ļ� %s ����ʧ�� �޷���\n", file_name);
		return 0;
	}

	// ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�	
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���	
	GLint line_bytes = width * 3;
	while (line_bytes % 4 != 0) {
		++line_bytes;
	}
	total_bytes = line_bytes * height;

	// �����������ֽ��������ڴ�
	pixels = (GLubyte*)malloc(total_bytes);
	if (0 == pixels) {
		fclose(pFile);
		printf_s("�ļ� %s ����ʧ�� ����Ϊ0\n", file_name);
		return 0;
	}

	// ��ȡ��������	
	if (fread(pixels, total_bytes, 1, pFile) <= 0) {
		free(pixels);
		fclose(pFile);
		printf_s("�ļ� %s ����ʧ�� ����ȱʧ\n", file_name);
		return 0;
	}

	// �Ծ;ɰ汾�ļ��ݣ����ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������
	// ��ͼ���߳�����OpenGL�涨�����ֵ��Ҳ����	
	GLint max;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
	if (!power_of_two(width)
		|| !power_of_two(height)
		|| width > max
		|| height > max)
	{
		const GLint new_width = 256;
		const GLint new_height = 256; // �涨���ź��µĴ�СΪ�߳���������		
		GLint new_line_bytes, new_total_bytes;
		GLubyte* new_pixels = 0;

		// ����ÿ����Ҫ���ֽ��������ֽ���	
		new_line_bytes = new_width * 3;
		while (new_line_bytes % 4 != 0) {
			++new_line_bytes;
		}
		new_total_bytes = new_line_bytes * new_height;

		// �����ڴ�		
		new_pixels = (GLubyte*)malloc(new_total_bytes);
		if (0 == new_pixels) {
			free(pixels);
			fclose(pFile);
			printf_s("�ļ� %s ����ʧ�� �ڴ����ʧ��\n", file_name);
			return 0;
		}

		// ������������		
		gluScaleImage(GL_RGB,
			width, height, GL_UNSIGNED_BYTE, pixels,
			new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

		// �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height		
		free(pixels);
		pixels = new_pixels;
		width = new_width;
		height = new_height;
	}

	//���ð�ɫΪ͸����0,֮ǰ��ȡ��RGB��������ҪRGBA
	unsigned char* texture = 0;
	texture = (unsigned char*)malloc(width * height * 4);
	for (int m = 0; m < width; m++) {
		for (int j = 0; j < height; j++) {
			//����ɫֵд��   	
			texture[m*width * 4 + j * 4] = pixels[m*width * 3 + j * 3];
			texture[m*width * 4 + j * 4 + 1] = pixels[m*width * 3 + j * 3 + 1];
			texture[m*width * 4 + j * 4 + 2] = pixels[m*width * 3 + j * 3 + 2];

			//����alphaֵ,�����ɫΪ͸��ɫ   
			if (texture[m*width * 4 + j * 4] >= 200
				&& texture[m*width * 4 + j * 4 + 1] >= 200
				&& texture[m*width * 4 + j * 4 + 2] >= 200) {
				texture[m*width * 4 + j * 4 + 3] = 0;//͸����alpha=0  
				//cout << (int)texture[m*width * 4 + j * 4 + 3];
			}
			else {
				texture[m*width * 4 + j * 4 + 3] = 255;//��͸����alpha=255  
				//cout << (int)texture[m*width * 4 + j * 4 + 3];
			}
		}
	}

	// ����һ���µ�������
	glGenTextures(1, &texture_ID);
	if (0 == texture_ID) {
		free(pixels);
		free(texture);
		fclose(pFile);
		printf_s("�ļ� %s ����ʧ�� �������ʧ��\n", file_name);
		return 0;
	}

	// ���µ������������������������
	// �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�	
	GLint lastTextureID = last_texture_ID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, lastTextureID);  //�ָ�֮ǰ�������
	free(pixels);
	free(texture);
	//printf_s("�ļ� %s ������� �ɹ�\n", file_name);
	return texture_ID;
}

//�ķ�����Ԫ
//F�������ɣ�X����Ҷ�ӣ�AB�������ɲ�����+-/*%&����任��[]����ջ
string grammar1 = "FA[+*&X][-/X][+%X]B";
string grammar2 = "FA[%-X][&*X][+/X][-*X]B";
string grammar3 = "FA[-*X][/&X][*%X][+&X]B";
string treegra;

//����Ԫ�����ķ��ַ���
string grammarIteration(string grammar, int level)
{
	string tempRule = grammar;
	string rule = {};
	for (int i = 1; i <= level; i++)
	{
		int curlen = tempRule.length();
		int j = 0;
		srand((int)time(0));//�������
		while (j < curlen) {
			if ('X' == tempRule[j]) { // X�������滻���ķ�ģ��		
				int n = random(3);
				//cout << n << endl;
				if (0 == n) {
					rule += grammar3;
				}
				else if (1 == n) {
					rule += grammar2;
				}
				else if (2 == n) {
					rule += grammar1;
				}
				j++;
			}
			else { // ����	
				rule += tempRule[j];
				j++;
			}
		}
		tempRule = rule;
		rule.clear();
	}
	rule = tempRule;
	return rule;
}

// ��������
void drawTree(float baseRadius, float topRadius, float height)
{
	GLUquadricObj *tree_obj;
	tree_obj = gluNewQuadric();//���������������
	if (tree != 0 && veinState == 1) {
		glBindTexture(GL_TEXTURE_2D, tree);//������
		gluQuadricTexture(tree_obj, GL_TRUE);
	}
	else {
		glColor3ub(155, 81, 33);//�������ʧ��ʱ����
	}
	gluCylinder(tree_obj, baseRadius, topRadius, height, 30, 30);//����
	glTranslatef(0.0f, 0.0f, height);
	gluSphere(tree_obj, topRadius, 30, 30);
	gluDeleteQuadric(tree_obj);
}
// ����Ҷ��
void drawLeaf()
{
	if (leaf != 0 && veinState == 1) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.9f);
		glBindTexture(GL_TEXTURE_2D, leaf);//������
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
		glTexCoord2f(1.0, 0.0); glVertex3f(10, 10, 0);
		glTexCoord2f(1.0, 1.0); glVertex3f(0, 20, 0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-10, 10, 0);
		glEnd();
		glDisable(GL_ALPHA_TEST);
	}
	else {
		glColor3f(1.0f, 0.0f, 0.0f);//�������ʧ��ʱ����
		glBegin(GL_QUADS);
		glVertex3f(0, 0, 0);
		glVertex3f(5, 5, 0);
		glVertex3f(0, 10, 0);
		glVertex3f(-5, 5, 0);
		glEnd();
	}
}
// �����ķ����Ʒ�����
void grammarDraw(string grammar)
{
	float baseRadius = 10, topRadius = 6, height = 200;
	int i = 0;
	while (i < grammar.length()) {
		switch (grammar[i]) {
			//F�������ɣ�X����Ҷ�ӣ�AB�������ɲ�����+-/*%&����任��[]����ջ
		case'F':drawTree(baseRadius, topRadius, height); break;
		case'X':drawLeaf(); break;
		case'A':height *= 0.6; baseRadius *= 0.5; topRadius *= 0.5; break;
		case'B':height /= 0.6; baseRadius /= 0.5; topRadius /= 0.5; break;
		case'+':glRotatef(35, 1, 0, 0); break;
		case'-':glRotatef(-35, 1, 0, 0); break;
		case'*':glRotatef(35, 0, 1, 0); break;
		case'/':glRotatef(-35, 0, 1, 0); break;
		case'%':glRotatef(35, 0, 0, 1); break;
		case'&':glRotatef(-35, 0, 0, 1); break;
		case'[':glPushMatrix(); break;
		case']':glPopMatrix(); break;
		}
		i++;
	}
}

// ���ƿռ䣨Pojectway=-1ʹ����ͶӰ������ʹ��͸��ͶӰ��
void DrawRoom(int PojectWay)
{
	glMatrixMode(GL_PROJECTION);//ͶӰ����Ӧ��
	glLoadIdentity();
	//����ͶӰ�ռ�
	if (-1 == PojectWay) {
		//��ͶӰ�ռ�
		glOrtho(-WinLong / 2, WinLong / 2, -WinWide / 2, WinWide / 2, -DISTANCE, DISTANCE);
	}
	else {
		//͸��ͶӰ�ռ�
		gluPerspective(70, WinLong / WinWide, 1, DISTANCE / 4);//��Ұ��Զ����distance/6����ȻԶ�����ضѻ���һ����
	}

	glMatrixMode(GL_MODELVIEW);	//ģ����ͼ����Ӧ��
	glLoadIdentity();

	//�趨���λ��
	glTranslatef(camera_x, camera_y, camera_z);//�ƶ�λ��
	gluLookAt(WinWide / 2, WinWide / 2, WinWide / 4, 0, 0, 0, 0, 0, 1);//�趨��λ
	glRotatef(univer_r, 0, 0, 1);//�趨��λ����תΪ��ת�ռ�

	//��������
	if (1 == lineState) {
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);//��ɫx��
		glVertex3f(-DISTANCE, 0, 0); glVertex3f(DISTANCE, 0, 0);

		glColor3f(0, 1, 0);//��ɫy��
		glVertex3f(0, -DISTANCE, 0); glVertex3f(0, DISTANCE, 0);

		glColor3f(0, 0.7, 1);//��ɫz��
		glVertex3f(0, 0, -DISTANCE); glVertex3f(0, 0, DISTANCE);

		glColor3f(0.5, 0.5, 0.5);
		for (int i = 0; i * 50 <= DISTANCE; i++) {
			glVertex2f(i * 50, -DISTANCE); glVertex2f(i * 50, DISTANCE);
			glVertex2f(-i * 50, -DISTANCE); glVertex2f(-i * 50, DISTANCE);
		}
		for (int i = 0; i * 50 <= DISTANCE; i++) {
			glVertex2f(-DISTANCE, i * 50); glVertex2f(DISTANCE, i * 50);
			glVertex2f(-DISTANCE, -i * 50); glVertex2f(DISTANCE, -i * 50);
		}
		glEnd();
	}

	//��Դ
	glEnable(GL_LIGHTING);//���õ�Դ
	GLfloat light_position[] = { 100, 100, 100, 1.0 };//w=1.0����λ��Դ
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glEnable(GL_LIGHT0);//�򿪵ƹ�1

	if (veinState == 1) {
		glEnable(GL_TEXTURE_2D);//������
	}

	// ���Ʒ�����
	grammarDraw(treegra);

	glLoadIdentity();
	if (veinState == 1) {
		glDisable(GL_TEXTURE_2D);//�ر�����
	}
	glDisable(GL_LIGHTING);//�رյ�Դ
}
// ��ʾ����
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//��ɫ����Ȼ������

	//������ͼ��
	glViewport(0, 0, WinLong, WinWide);
	DrawRoom(projectWay);

	glutSwapBuffers();   //˫����ˢ��
}
// ��ʼ������ 
void Initial(void)
{
	//������ʼ��
	int starGrammar = random(3);
	if (0 == starGrammar) {
		treegra = grammarIteration(grammar1, level);
	}
	else if (1 == starGrammar) {
		treegra = grammarIteration(grammar2, level);
	}
	else if (2 == starGrammar) {
		treegra = grammarIteration(grammar3, level);
	}
	cout << "�ķ�Ϊ��" << treegra << endl << endl;

	glEnable(GL_COLOR_MATERIAL);//������ϣ��ģ�͵���ɫ����������
	glEnable(GL_DEPTH_TEST);//������Ȳ��ԣ�GL�����ǰ�����˳����Ƶģ��������������ڵ���ϵ
	glDepthFunc(GL_LESS);//��ʵ�����е�ǰ��

	glClearColor(0.8f, 0.8f, 0.8f, 0.1f);//������ǰ3����RGB�������Alphaֵ������͸����0.0��ʾ��ȫ͸��
	//���ƺ����з�������ͬͶӰ��ʽ���ƣ����ڳ�ʼ��������ͶӰ�ռ�
	
	leaf = LoadTexture("leaf.bmp");
	tree = LoadTexture("tree.bmp");//��ȡbmp�ļ���������
}

// ���̻ص�����
void keyBoard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q':
		camera_x = 0;
		camera_y = -200;
		camera_z = 0;
		univer_r = 0;
		break;
	case 'e':projectWay *= -1; break;
		//�������xyz���ƶ��������꣬���Ч�����൱����������������� �� �����ƶ�
	case 'w':camera_z += 10; break;
	case 's':camera_z -= 10; break;
	case 'a':camera_x += 10; break;
	case 'd':camera_x -= 10; break;

	case 'z':univer_r += 1; break;
	case 'c':univer_r -= 1; break;

	case '1':level = 5; cout << "����������" << level << endl; Initial(); break;
	case '2':lineState *= -1; break;
	case '3':veinState *= -1; break;

	case'4':level--; cout << "����������" << level << endl; Initial(); break;
	case'5':level++; cout << "����������" << level << endl; Initial(); break;
	}
	glutPostRedisplay();  //�ػ������µ���Display()
}
// ���ص�
void onMouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0) {
		camera_y -= 7.5;
	}
	if (dir < 0) {
		camera_y += 7.5;
	}
	glutPostRedisplay();  //�ػ������µ���Display()
}

// ʹ��glut�⺯�����д��ڹ���
int main(int argc, char * argv[])
{
	//������ʼ��
	camera_x = 0;
	camera_y = -200;
	camera_z = 0;
	lineState = 1;
	veinState = 1;
	projectWay = -1;
	level = 5;

	printf_s("����˵������رմ�д���룬tree/leaf.bmp�ļ����exe����ͬһĿ¼�£�����Ҳû�� ");
	printf_s("\n\n�������������� >= 8\n");
	printf_s("\n     1 - ��������5������     2 - ������    3 - ������         4 - ���ٵ�������     5 - ���ӵ�������");
	printf_s("\n     q - ���س�ʼ��λ                   e - �л�ͶӰ��ʽ  ");
	printf_s("\n                        w - ǰ��                          ");
	printf_s("\n     a - ����           s - ����        d - ����          ");
	printf_s("\n     z - �ռ�˳ʱ��ת                   c - �ռ���ʱ��ת  ");
	printf_s("\n                        x - �ر�/������                 ");
	printf_s("\n    ���������Ϲ��� - ����            ���������¹��� - ����\n\n");

	//���ڳ�ʼ��
	glutInit(&argc, argv);//glut��ʼ��
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);//RGBA��ɫģ��|˫����
	glutInitWindowPosition(0, 100);//��ʼλ��
	glutInitWindowSize(WinLong, WinWide);//���ڴ�С

	glutCreateWindow("��ġ�L������");//��������

	Initial();//��ʼ��
	glutDisplayFunc(&myDisplay);//myDisplayָ��Ϊ��ǰ���ڵ���ʾ���ݺ���

	//ע��ص�����
	glutKeyboardFunc(&keyBoard);
	glutMouseWheelFunc(&onMouseWheel);
	glutMainLoop();//ʹ���ڿ������

	return 0;
}

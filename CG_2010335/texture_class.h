
#pragma once
#pragma warning(disable : 4996)

class texture_class
{
private:
	std::string m_fileName;
	int m_width;
	int m_height;

protected:
	GLuint texture_id;    // �e�N�X�`��ID

public:
	texture_class(std::string fileName, int width, int height) : m_fileName(fileName)
	{
		texture_id = -1;
		m_width = width;
		m_height = height;
	}

	void init()
	{
		int size = m_height * m_width * 4;
		std::vector<GLubyte> texture_buf(size);
		FILE* fp;
		/* �e�N�X�`���摜(1����)�̓ǂݍ��� */
		if ((fp = fopen(m_fileName.c_str(), "rb")) != NULL) {
			fread(texture_buf.data(), sizeof (GLubyte) * size, 1, fp);
			fclose(fp);
		}
		else {
			perror(m_fileName.c_str());
		}

		// �e�N�X�`���̓ǂݍ���
		glGenTextures(1, &texture_id);  // �e�N�X�`��ID�𐶐�
		glBindTexture(GL_TEXTURE_2D, texture_id);  // ���������e�N�X�`��ID�ɐ؂�ւ���

		/* �e�N�X�`���摜�̓o�C�g�P�ʂɋl�ߍ��܂�Ă��� */
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		/* �e�N�X�`���̊��蓖�� */
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, m_width, m_height,  // �c���T�C�Y�̓e�N�X�`��1�p
			GL_RGBA, GL_UNSIGNED_BYTE, texture_buf.data());

		/* �e�N�X�`�����g��E�k��������@�̎w�� */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		/* �e�N�X�`���� */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glBindTexture(GL_TEXTURE_2D, 0);  // �f�t�H���g�Ƀe�N�X�`��ID�ɐ؂�ւ���
	}
};

class sea_plane : public texture_class
{
public:
	sea_plane(std::string fileName, int width, int height) : texture_class(fileName, width, height) {}

	void scene()
	{
		/* �ގ��̐ݒ� */
		const GLfloat color[] = { 1.0, 1.0, 1.0, 1.0 };  /* �ގ� (�F) */
		const GLfloat black[] = { 0, 0, 0, 1 };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
		glMaterialfv(GL_FRONT, GL_AMBIENT, black);
		glMaterialfv(GL_FRONT, GL_SPECULAR, black);
		glMaterialfv(GL_FRONT, GL_SHININESS, black);

		/* �A���t�@�e�X�g�J�n */
		glEnable(GL_ALPHA_TEST);

		/* �e�N�X�`���}�b�s���O�J�n */
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, texture_id);  // �e�N�X�`��ID=1���w��

		const float extent = 2.0f;    // -extent����extent�܂Ń^�C����\��
		const float split = 0.5f;    // �^�C���̑傫��
		float tex_x, tex_y;
		for (tex_x = -extent; tex_x < extent; tex_x += split)
		{
			for (tex_y = -extent; tex_y < extent; tex_y += split)
			{
				/* �P���̂S�p�`��`�� */
				//glNormal3d(0.0, 1.0, 0.0);
				glBegin(GL_QUADS);
				glTexCoord2d(0.0, 1.0);
				glVertex3d(tex_x, 0.0, tex_y);
				glTexCoord2d(1.0, 1.0);
				glVertex3d(tex_x + split, 0.0, tex_y);
				glTexCoord2d(1.0, 0.0);
				glVertex3d(tex_x + split, 0.0, tex_y + split);
				glTexCoord2d(0.0, 0.0);
				glVertex3d(tex_x, 0.0, tex_y + split);
				glEnd();
			}
		}
		
		glBindTexture(GL_TEXTURE_2D, texture_id);  // �e�N�X�`��ID���f�t�H���g�ɖ߂�

		/* �e�N�X�`���}�b�s���O�I�� */
		glDisable(GL_TEXTURE_2D);

		/* �A���t�@�e�X�g�I�� */
		glDisable(GL_ALPHA_TEST);
	}
};

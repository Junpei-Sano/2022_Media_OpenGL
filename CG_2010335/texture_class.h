
#pragma once
#pragma warning(disable : 4996)

class texture_class
{
private:
	std::string m_fileName;
	int m_width;
	int m_height;

protected:
	GLuint texture_id;    // テクスチャID

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
		/* テクスチャ画像(1枚目)の読み込み */
		if ((fp = fopen(m_fileName.c_str(), "rb")) != NULL) {
			fread(texture_buf.data(), sizeof (GLubyte) * size, 1, fp);
			fclose(fp);
		}
		else {
			perror(m_fileName.c_str());
		}

		// テクスチャの読み込み
		glGenTextures(1, &texture_id);  // テクスチャIDを生成
		glBindTexture(GL_TEXTURE_2D, texture_id);  // 生成したテクスチャIDに切り替える

		/* テクスチャ画像はバイト単位に詰め込まれている */
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		/* テクスチャの割り当て */
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, m_width, m_height,  // 縦横サイズはテクスチャ1用
			GL_RGBA, GL_UNSIGNED_BYTE, texture_buf.data());

		/* テクスチャを拡大・縮小する方法の指定 */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		/* テクスチャ環境 */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glBindTexture(GL_TEXTURE_2D, 0);  // デフォルトにテクスチャIDに切り替える
	}
};

class sea_plane : public texture_class
{
public:
	sea_plane(std::string fileName, int width, int height) : texture_class(fileName, width, height) {}

	void scene()
	{
		/* 材質の設定 */
		const GLfloat color[] = { 1.0, 1.0, 1.0, 1.0 };  /* 材質 (色) */
		const GLfloat black[] = { 0, 0, 0, 1 };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
		glMaterialfv(GL_FRONT, GL_AMBIENT, black);
		glMaterialfv(GL_FRONT, GL_SPECULAR, black);
		glMaterialfv(GL_FRONT, GL_SHININESS, black);

		/* アルファテスト開始 */
		glEnable(GL_ALPHA_TEST);

		/* テクスチャマッピング開始 */
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, texture_id);  // テクスチャID=1を指定

		const float extent = 2.0f;    // -extentからextentまでタイルを貼る
		const float split = 0.5f;    // タイルの大きさ
		float tex_x, tex_y;
		for (tex_x = -extent; tex_x < extent; tex_x += split)
		{
			for (tex_y = -extent; tex_y < extent; tex_y += split)
			{
				/* １枚の４角形を描く */
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
		
		glBindTexture(GL_TEXTURE_2D, texture_id);  // テクスチャIDをデフォルトに戻す

		/* テクスチャマッピング終了 */
		glDisable(GL_TEXTURE_2D);

		/* アルファテスト終了 */
		glDisable(GL_ALPHA_TEST);
	}
};

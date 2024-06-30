
#pragma once

#define M_PI 3.14159265358979323846

class camera_rotate
{
private:
	GLdouble m_target[3];    // ���_�̑Ώۂ̓_(x, y, z)
	float m_ry, m_rxz;    // ��]�iy�����Axz���ʏ�̎����j
	float m_r;            // ��]���a

	float m_angle_step;
	float m_move_step;
	float m_zoom_step;

	int m_mouse_state = 0;
	int m_prev_x, m_prev_y;

	GLdouble m_currentPos[3];    // �J�����̌��݈ʒu���L�^�i���E�ړ��̂��߁j

	// �O�ς��v�Z����
	// �v�Z��͑傫����1��
	void cross_normalize(GLdouble ans[], const GLdouble v1[], const GLdouble v2[])
	{
		// �O�όv�Z
		ans[0] = v1[1] * v2[2] - v1[2] * v2[1];
		ans[1] = v1[2] * v2[0] - v1[0] * v2[2];
		ans[2] = v1[0] * v2[1] - v1[1] * v2[0];
		// ���K��
		double len = sqrt(ans[0] * ans[0] + ans[1] * ans[1] + ans[2] * ans[2]);
		ans[0] /= len; ans[1] /= len; ans[2] /= len;
	}

	void set_angle_diff(float dx, float dy)
	{
		m_ry += dx * m_angle_step;
		m_rxz += dy * m_angle_step;
	}

	void set_position_diff(float dx, float dy)
	{
		GLdouble fwd[] = { m_currentPos[0], m_currentPos[1], m_currentPos[2] };
		GLdouble up[] = { 0, 1, 0 };
		// �O�ς��牡�������v�Z
		GLdouble side[3];
		cross_normalize(side, fwd, up);
		m_target[0] += side[0] * dx * m_move_step * m_r;    // m_r���|���ĉ����قǑ����ړ�
		m_target[1] += dy * m_move_step * m_r;
		m_target[2] += side[2] * dx * m_move_step * m_r;
	}

	void set_zoom_diff(float dy)
	{
		m_r += dy * m_zoom_step;
		if (m_r < 0.1f) { m_r = 0.1f; }
	}

public:
	camera_rotate()
	{
		m_target[0] = m_target[1] = m_target[2] = 0.0;
		m_ry = (float)M_PI / 2;
		m_rxz = (float)M_PI / 6;
		m_r = 7;

		m_angle_step = (float)M_PI / 180 / 10;
		m_zoom_step = 0.01f;
		m_move_step = 0.001f;

		m_prev_x = m_prev_y = 0;

		m_currentPos[0] = m_currentPos[1] = m_currentPos[2] = .0;
	}

	// 0�F�Ȃɂ�������Ă��Ȃ�
	// 1�F��]
	// 2�F�ړ�
	// 3�F�g��k��
	void set_mouse_status(int state, int x, int y)
	{
		m_mouse_state = state;
		m_prev_x = x; m_prev_y = y;
	}

	void set(int x, int y)
	{
		int dx = x - m_prev_x;
		int dy = y - m_prev_y;
		switch (m_mouse_state)
		{
		case 1:
			set_angle_diff(dx, dy);
			break;
		case 2:
			set_position_diff(dx, dy);
			break;
		case 3:
			set_zoom_diff(dy);
			break;
		}
		m_prev_x = x; m_prev_y = y;
	}

	void move()
	{
		GLdouble y = m_r * sin(m_rxz);
		double rxz = m_r * cos(m_rxz);
		GLdouble x = rxz * cos(m_ry);
		GLdouble z = rxz * sin(m_ry);
		gluLookAt(x+m_target[0], y+m_target[1], z+m_target[2], m_target[0], m_target[1], m_target[2], 0.0, 1.0, 0.0);
		m_currentPos[0] = x; m_currentPos[1] = y; m_currentPos[2] = z;
	}
};
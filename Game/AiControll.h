#pragma once

#include "../Engine/Glider/Glider.h"
#include "../Engine/Common/IncludesMatem.h"

class AiControll : public AIInterface
{
public:
	inline void setCommand(GliderCommand command)
	{
		if (!_glider)
			return;

		_glider->getCommands()[command] = true;
	}

	inline void setVector(const glm::vec3& vector)
	{
		_lookVector = vector;
		_lookVector.z = 0.0f;
		glm::normalize(_lookVector);
		_glider->setLookVector(_lookVector);
	}

private:
	glm::vec3 _lookVector = glm::vec3(1.0f, 0.0f, 0.0f);
};

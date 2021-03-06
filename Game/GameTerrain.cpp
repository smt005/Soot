#include "GameTerrain.h"
#include "AiControll.h"
#include "AiTarget.h"

#include "../App/App.h"
#include "../App/File.h"

#include "../Engine/Draw/DrawEngine.h"
#include "../Engine/Draw/Camera.h"
#include "../Engine/Object/Map.h"
#include "../Engine/Object/Object.h"
#include "../Engine/Object/Model.h"
#include "../Engine/Object/Shape.h"
#include "../Engine/Glider/Glider.h"
#include "../Engine/Glider/GliderTemplate.h"
#include "../Engine/Glider/Gun.h"
#include "../Engine/Effect/EffectObject.h"
#include "../Engine/Common/Help.h"
#include "../Engine/Common/IncludesMatem.h"

#include "../Engine/Common/Log.h"

GameTerrain::GameTerrain()
{
}

GameTerrain::~GameTerrain()
{
}

bool GameTerrain::init()
{
	initMap();
	initDraw();
	initCallback();

	//testFunction();

	return true;
}

void GameTerrain::save()
{
}

void GameTerrain::update()
{
	_mapPtr->action();

	Glider& glider = _mapPtr->getGliderByName("Glider_player");
	vec3 pos = glider.getPos();
	pos.x -= 4.0f;
	pos.z += 5.0f;
	Camera::current.setPos(pos);

	vec3 posCursor = Camera::current.corsorCoord();

	Object& object = help::find(_mapPtr->getObjects(), "aim");
	object.setPos(posCursor);

	if (_ai)
	{
		Glider* glider = _ai->getGlider();
		if (glider)
		{
			vec3 posGlider = glider->getPos();
			vec3 vector(normalize(posCursor - posGlider));
			_ai->setVector(vector);
		}
	}
}

void GameTerrain::draw()
{
	DrawEngine::prepareDraw(true);

	if (!_visiblePhysic)
	{
		DrawEngine::drawMap(*_mapPtr);
        
        if (_visibleVectorLight)
        {
            DrawEngine::prepareDrawLine();
            vector<Object*>& objects = _mapPtr->_objects;

            for (auto object : objects)
            {
                const glm::mat4x4& mat = object->getMatrix();

                float posObject[] = { mat[3][0], mat[3][1] , mat[3][2] };
                const float* lightDirect = DrawEngine::getLightDirect();
                float lenghtLine = 10.0f;
                float lightVector[] = { (posObject[0] - lightDirect[0] * lenghtLine), (posObject[1] - lightDirect[1] * lenghtLine), (posObject[2] - lightDirect[2] * lenghtLine) };
                float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                DrawEngine::drawLine(posObject, lightVector, color);
            }
        }
	}
	else
	{
		DrawEngine::drawMapPhysic(*_mapPtr);
	}
}

void GameTerrain::initMap()
{
	_mapPtr = Map::getByName("MapGameTerrain");
	Map::setCurrent(_mapPtr);
	_mapPtr->setPhysic();
	Physics::setGravity(vec3(0.0f, 0.0f, 0.0f));

	initGlider();
}

void GameTerrain::initGlider()
{
	// Enemy

	vector<Glider*>& gliders = _mapPtr->getGliders();
	for_each(gliders.begin(), gliders.end(), [](Glider* glider)
	{
		if (!glider)
			return;

		if (glider->name() != "Glider_player")
		{
			AiTarget* ai = new AiTarget();
			glider->setAi(ai);
		}
	}
	);

	//Player

	Glider& glider = _mapPtr->getGliderByName("Glider_player");
	glider.setTemplate(GliderTemplate::getByName("attacker"));
	glider.setPos(vec3(0.0f, 0.0f, 3.0f));

	_ai = new AiControll();
	glider.setAi(_ai);

	Object& object = _mapPtr->addObject("aim_3d");
	object.setName("aim");
}

void GameTerrain::initDraw()
{
	DrawEngine::setBackgroundColor(0.3f, 0.6f, 0.9f, 1.0f);
	DrawEngine::setAmbientColor(0.2f, 0.2f, 0.3f);
	DrawEngine::initDrawMap();
	DrawEngine::initDrawLines();

	Camera::current.setDefault();
	Camera::current.setSpeed(1.0f);
	Camera::current.setFromEye(false);
	Camera::current.setDist(5.0f);
	Camera::current.setVector(vec3(1.0f, 0.0f, -1.0f));
}

void GameTerrain::initCallback()
{
	//addCallback(EventCallback::TAP_PINCH, Function(rotateCamera));
	//addCallback(EventCallback::MOVE, Function(rotateCamera));

	addCallback(EventCallback::TAP_PINCH, Function(shoot));
	addCallback(EventCallback::BUTTON_UP, Function(pressButton));
	addCallback(EventCallback::BUTTON_PINCH, Function(pressButtonPinch));
	addCallback(EventCallback::BUTTON_DOWN, Function(pressButtonDown));
}

bool GameTerrain::close(void* data)
{
	App::close();
	return true;
}

bool GameTerrain::shoot(void *data)
{
	if (_ai)
		_ai->setCommand(GliderCommand::SHOOT);

	return true;
}

bool GameTerrain::rotateCamera(void *data)
{
	Camera::current.rotate(Callback::vector);
	return true;
}

bool GameTerrain::pressButton(void *data)
{
	if (Callback::charButtonUp == VK_ESCAPE)
	{
		App::close();
		return true;
	}

	if (Callback::charButtonUp == VK_SPACE)
	{
		addObject("Dumbbell_05");
		return true;
	}

    if (Callback::charButtonUp == 'L')
    {
        _visibleVectorLight = !_visibleVectorLight;
    }
    
	if (Callback::charButtonUp == 'T')
	{
		int countGenerateGlider = 5;
		for (int i = 0; i < countGenerateGlider; ++i)
		{
			AiTarget* ai = new AiTarget();
			vec3 pos(
				help::random_f(-5.0f, 5.0f),
				help::random_f(-5.0f, 5.0f),
				1.0f
			);

			_mapPtr->addGlider("attacker", ai, pos);
		}
	}

	if (Callback::charButtonUp == 'P')
	{
		vec3 a(3.0f, 2.0f, 1.0f);
		vec3 b(4.0f, 5.0f, 6.0f);
		vec3 c(9.0f, 8.0f, 7.0f);

		PhysicPlane plane;
		plane.set(a, b, c);

		float area = plane._area;

		vec3 point(2.0f, 6.0f, 4.0f);
		float dist = plane.distPointToPlane(point);

		LOGI("LOG: area = %f", area);
		LOGI(" dist = %f\n", dist);
	}

	return false;
}

bool GameTerrain::pressButtonPinch(void *data)
{
	controlGlider();
	controllCamera();

	return true;
}

bool GameTerrain::pressButtonDown(void *data)
{
	if (Callback::charButtonDown == VK_SPACE)
	{
		addObject("Dumbbell_0125");
		return true;
	}


	if (Callback::key[VK_CONTROL] && Callback::key['P'])
	{
		_visiblePhysic = !_visiblePhysic;
	}

	return false;
}

void GameTerrain::addObject(const string& name)
{
	glm::vec3 randomPos;
	randomPos.x = help::random_f(-50.0f, 100.0f);
	randomPos.y = help::random_f(-50.0f, 100.0f);
	randomPos.z = help::random_f(10.0f, 100.0f);

	_mapPtr->addObjectToPos(name, PhysicType::CONVEX, randomPos);
}

void GameTerrain::controlGlider()
{
	if (Callback::key[VK_CONTROL])
		return;

	if (!_ai)
		return;

	if (Callback::key['W'])
	{
		_ai->setCommand(GliderCommand::FOWARD_VIEW);
	}

	if (Callback::key['S'])
	{
		_ai->setCommand(GliderCommand::BACK_VIEW);
	}

	if (Callback::key['A'])
	{
		_ai->setCommand(GliderCommand::LEFT_VIEW);
	}

	if (Callback::key['D'])
	{
		_ai->setCommand(GliderCommand::RIGHT_VIEW);
	}

	if (Callback::key['R'])
	{
	}

	if (Callback::key['F'])
	{

	}
}

void GameTerrain::controllCamera()
{
	if (!Callback::key[VK_CONTROL])
		return;

	float speedCamera = Callback::key[VK_SHIFT] ? 1.0f : speedCamera = 0.125f;
	Camera::current.setSpeed(speedCamera);

	if (Callback::key['W'])
	{
		Camera::current.move(CameraMove::FORVARD);
	}

	if (Callback::key['S'])
	{
		Camera::current.move(CameraMove::BACK);
	}

	if (Callback::key['A'])
	{
		Camera::current.move(CameraMove::RIGHT);
	}

	if (Callback::key['D'])
	{
		Camera::current.move(CameraMove::LEFT);
	}

	if (Callback::key['R'])
	{
		Camera::current.move(CameraMove::TOP);
	}

	if (Callback::key['F'])
	{
		Camera::current.move(CameraMove::DOWN);
	}
}

class ClassTestBase
{
	string strB;
public:
	virtual void fB() {};
};

class ClassTest
{

};

void GameTerrain::testFunction()
{
	LOG("\n");

	LOG_SIZE_STRUCT("ClassTestBase", ClassTestBase());
	LOG_SIZE_STRUCT("ClassTest", ClassTest());

	LOG("\n");

	LOG_SIZE_STRUCT("int", int());
	LOG_SIZE_STRUCT("double", double());

	LOG("\n");

	LOG_SIZE_STRUCT("string", string());
	LOG_SIZE_STRUCT("vec3", vec3());
	LOG_SIZE_STRUCT("mat4x4", mat4x4());
	
	LOG("\n");

	LOG_SIZE_STRUCT("ShapePtr", ShapePtr(new Shape()));
	LOG_SIZE_STRUCT("shared_ptr<Shape>", shared_ptr<Shape>(new Shape()));
	LOG_SIZE_STRUCT("shared_ptr<int>", shared_ptr<int>(new int()));

	LOG("\n");

	LOG_SIZE_STRUCT("Texture", Texture());
	LOG_SIZE_STRUCT("MeshPhysic", MeshPhysic());
	LOG_SIZE_STRUCT("PhysicObject", PhysicObject());
	LOG_SIZE_STRUCT("Model", Model());
	LOG_SIZE_STRUCT("Mesh", Mesh());
	LOG_SIZE_STRUCT("Shape", Shape());
	LOG_SIZE_STRUCT("Object", Object());
	LOG_SIZE_STRUCT("Glider", Glider());
	LOG_SIZE_STRUCT("Gun", Gun());
	LOG_SIZE_STRUCT("Map", Map());

	LOG("\n");
}
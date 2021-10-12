#pragma once

#include "Engine/System.hpp"

#include "GraphicPipeline.hpp"

class Graphic : public System
{
public:
	Graphic(VkDevice device);

	void init() override;
	void update(float dt) override;
	void close() override;
private:
	GraphicPipeline* graphicPipeline = nullptr;
};
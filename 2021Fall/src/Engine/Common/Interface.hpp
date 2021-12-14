#pragma once

//All classes should inherit this base class
class Interface
{
protected:
	Interface() {}

	//copy constructor
	Interface(const Interface& target) = delete;
	//move constructor
	Interface(Interface&& target) noexcept = delete;
	
	//copy assign operator
	Interface& operator=(Interface& target) = delete;
	//move assign operator
	Interface& operator=(Interface&& target) noexcept = delete;

public:
	//destructor
	virtual ~Interface() = default;

public:
	virtual void init() = 0;
	virtual void postinit() = 0;

	virtual void update(float dt) = 0;
	virtual void close() = 0;
};
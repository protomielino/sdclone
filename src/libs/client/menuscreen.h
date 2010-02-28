#pragma once


class MenuScreen
{
public:
	MenuScreen()
	{
		m_pMenuHandle = NULL;

	}
	void * GetMenuHandle(){return m_pMenuHandle;}
	void RunMenu(){GfuiScreenActivate(m_pMenuHandle);}

protected:
	void * m_pMenuHandle;
};

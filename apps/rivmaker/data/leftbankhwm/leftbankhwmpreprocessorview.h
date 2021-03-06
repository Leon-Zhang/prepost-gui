#ifndef LEFTBANKHWMPREPROCESSORVIEW_H
#define LEFTBANKHWMPREPROCESSORVIEW_H

#include "../points/pointspreprocessorview.h"

class LeftBankHWM;

class LeftBankHWMPreProcessorView : public PointsPreProcessorView
{
public:
	LeftBankHWMPreProcessorView(Model* model, LeftBankHWM* item);
	~LeftBankHWMPreProcessorView();

private:
	void drawMarker(const QPointF& position, QPainter* painter) const override;
	void doDraw(QPainter *painter) const override;
};

#endif // LEFTBANKHWMPREPROCESSORVIEW_H

class LoopView : public AbstractMenu::CustomItem
{
  private:
    FontDef GetValueFont(const char*      textToDraw,
                         const Rectangle& availableSpace) const
    {
        (void)(textToDraw); // ignore unused variable warning
        if(availableSpace.GetHeight() < 10)
            return Font_6x8;
        else if(availableSpace.GetHeight() < 18)
            return Font_7x10;
        else
            return Font_11x18;
    }
    Rectangle DrawLRArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                        Rectangle              topRowRect,
                                        bool                   leftAvailable,
                                        bool                   rightAvailable)
    {
        auto leftArrowRect = topRowRect.RemoveFromLeft(9)
                                 .WithSizeKeepingCenter(5, 9)
                                 .Translated(0, -1);
        auto rightArrowRect = topRowRect.RemoveFromRight(9)
                                  .WithSizeKeepingCenter(5, 9)
                                  .Translated(0, -1);

        if(leftAvailable)
        {
            for(int16_t x = leftArrowRect.GetRight() - 1;
                x >= leftArrowRect.GetX();
                x--)
            {
                display.DrawLine(x,
                                 leftArrowRect.GetY(),
                                 x,
                                 leftArrowRect.GetBottom(),
                                 true);

                leftArrowRect = leftArrowRect.Reduced(0, 1);
                if(leftArrowRect.IsEmpty())
                    break;
            }
        }
        if(rightAvailable)
        {
            for(int16_t x = rightArrowRect.GetX();
                x < rightArrowRect.GetRight();
                x++)
            {
                display.DrawLine(x,
                                 rightArrowRect.GetY(),
                                 x,
                                 rightArrowRect.GetBottom(),
                                 true);

                rightArrowRect = rightArrowRect.Reduced(0, 1);
                if(rightArrowRect.IsEmpty())
                    break;
            }
        }

        return topRowRect;
    };
    Rectangle DrawUDArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                        Rectangle              topRowRect,
                                        bool                   upAvailable,
                                        bool                   downAvailable)
    {
        auto upArrowRect
            = topRowRect.RemoveFromLeft(9).WithSizeKeepingCenter(9, 5);
        auto downArrowRect
            = topRowRect.RemoveFromRight(9).WithSizeKeepingCenter(9, 5);

        if(upAvailable)
        {
            for(int16_t y = upArrowRect.GetBottom() - 1;
                y >= upArrowRect.GetY();
                y--)
            {
                display.DrawLine(
                    upArrowRect.GetX(), y, upArrowRect.GetRight(), y, true);

                upArrowRect = upArrowRect.Reduced(1, 0);
                if(upArrowRect.IsEmpty())
                    break;
            }
        }
        if(downAvailable)
        {
            for(int16_t y = downArrowRect.GetY(); y < upArrowRect.GetBottom();
                y++)
            {
                display.DrawLine(
                    downArrowRect.GetX(), y, downArrowRect.GetRight(), y, true);

                downArrowRect = downArrowRect.Reduced(1, 0);
                if(downArrowRect.IsEmpty())
                    break;
            }
        }

        return topRowRect;
    };
    void DrawTopRow(OneBitGraphicsDisplay& display,
                    bool                   isVertical,
                    int                    currentIndex,
                    int                    numItemsTotal,
                    const char*            text,
                    Rectangle              rect,
                    bool                   isSelected)
    {
        const bool hasPrev = currentIndex > 0;
        const bool hasNext = true; //currentIndex < numItemsTotal - 1;
        // draw the arrows
        if(isSelected)
        {
            if(!isVertical)
                rect = DrawLRArrowsAndGetRemRect(
                    display, rect, hasPrev, hasNext);
            else
                rect = DrawUDArrowsAndGetRemRect(
                    display, rect, hasPrev, hasNext);
        }
        const auto font = GetValueFont(text, rect);
        display.WriteStringAligned(text, font, rect, Alignment::centered, true);
    }

  public:
    bool canBeEnteredForEditing_ = true;
    bool CanBeEnteredForEditing() const override
    {
        return canBeEnteredForEditing_;
    }

    int  loopRecorded = 0;
    void SetLoopRecorded(int loopRecorded)
    {
        this->loopRecorded = loopRecorded / 16;
    }
    int  GetLoopRecorded() { return this->loopRecorded; }
    bool editing = false;
    bool GetEditing() { return this->editing; }

    void Draw(daisy::OneBitGraphicsDisplay& display,
              int                           currentIndex,
              int                           numItemsTotal,
              daisy::Rectangle              boundsToDrawIn,
              bool                          isEditing) override
    {
        display.Fill(false);
        this->editing              = isEditing;
        auto       remainingBounds = display.GetBounds();
        const auto topRowHeight    = remainingBounds.GetHeight() / 2;
        const auto topRowRect = remainingBounds.RemoveFromTop(topRowHeight);

        DrawTopRow(display,
                   false,
                   currentIndex,
                   numItemsTotal,
                   "Loop",
                   topRowRect,
                   !isEditing);
        //auto checkboxBounds = remainingBounds.WithSizeKeepingCenter(12, 12);
        for(int i = 0; i < CROSSFADER_RESOLUTION; i++)
        {
            int spacing   = 0;
            int itemWidth = 128 / CROSSFADER_RESOLUTION;
            // int x         = i % (itemWidth / 2);
            // int y         = i / (itemWidth/2);
            int width  = itemWidth - spacing;
            int height = 8;

            int x_pos = i * (width + spacing);
            int y_pos = 32;

            Rectangle rect = Rectangle(x_pos, y_pos, width, height);

            display.DrawRect(rect, true);
            if(crossFaderPos > i)
            {
                display.DrawRect(rect.Reduced(1), true, true);
            }
        }

        for(int i = 0; i < this->loopLength; i++)
        {
            int spacing   = 2;
            int itemWidth = 128 / this->loopLength;
            // int x         = i % (itemWidth / 2);
            // int y         = i / (itemWidth/2);
            int width  = itemWidth - spacing;
            int height = 9;

            int       x_pos = i * (width + spacing);
            int       y_pos = 46;
            Rectangle rect  = Rectangle(x_pos, y_pos, width, height);

            display.DrawRect(rect, true);
            if(loopRecorded > i)
            {
                display.DrawRect(rect.Reduced(1), true, true);
            }
        }
        for(int i = 0; i < 16; i++)
        {
            int       width   = 6;
            int       height  = 4;
            int       spacing = 2;
            int       x_pos   = i * (width + spacing);
            int       y_pos   = 58;
            Rectangle rect    = Rectangle(x_pos, y_pos, width, height);

            display.DrawRect(rect, true);
            if(currentStep >= i)
            {
                display.DrawRect(rect.Reduced(1), true, true);
            }
        }

        display.Update();
    }

    int  currentStep = 0;
    void SetCurrentStep(int step) { this->currentStep = step; }
    int  GetCurrenStep() { return this->currentStep; }

    int  loopLength = 64;
    void SetLoopLength(int loopLength) { this->loopLength = loopLength / 16; }
    int  GetLoopLength() { return this->loopLength; }

    // 0 is left
    // 16 is middle
    // 32 is right
    int  crossFaderPos = 0;
    void SetCrossFaderPos(int crossFaderPos)
    {
        this->crossFaderPos = crossFaderPos;
    }
    int GetCrossFaderPos() { return this->crossFaderPos; }


    bool     modifySteppedCalled_                       = false;
    int16_t  incrementsPassedIntoModifyStepped_         = 0;
    uint16_t stepsPerRevolutionPassedIntoModifyStepped_ = 0;
    bool     isFuncButtonDownPassedIntoModifyStepped_   = false;
    void     ModifyValue(int16_t  increments,
                         uint16_t stepsPerRevolution,
                         bool     isFunctionButtonPressed) override
    {
        modifySteppedCalled_                       = true;
        incrementsPassedIntoModifyStepped_         = increments;
        stepsPerRevolutionPassedIntoModifyStepped_ = stepsPerRevolution;
        isFuncButtonDownPassedIntoModifyStepped_   = isFunctionButtonPressed;
    };

    bool  modifyContinuousCalled_                     = false;
    float valuePassedIntoModifyContinuous             = 0.0f;
    bool  isFuncButtonDownPassedIntoModifyContinuous_ = false;
    void  ModifyValue(float valueSliderPosition0To1,
                      bool  isFunctionButtonPressed) override
    {
        modifyContinuousCalled_                     = true;
        valuePassedIntoModifyContinuous             = valueSliderPosition0To1;
        isFuncButtonDownPassedIntoModifyContinuous_ = isFunctionButtonPressed;
    };

    bool onOkayButtonCalled_ = false;
    void OnOkayButton() override { onOkayButtonCalled_ = true; }
};
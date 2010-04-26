#include "zoom_gui_command.h"
#include "../application_context.h"

#include <QInputDialog>

namespace BioSig_
{

//-----------------------------------------------------------------------------
QString const ZoomGuiCommand::GOTO_ = "Go to...";
QString const ZoomGuiCommand::ZOOM_IN_VERTICAL_ = "Zoom In Vertical";
QString const ZoomGuiCommand::ZOOM_OUT_VERTICAL_ = "Zoom Out Vertical";
QString const ZoomGuiCommand::ZOOM_IN_HORIZONTAL_ = "Zoom In Horizontal";
QString const ZoomGuiCommand::ZOOM_OUT_HORIZONTAL_ = "Zoom Out Horizontal";


QStringList const ZoomGuiCommand::ACTIONS_ = QStringList() << ZoomGuiCommand::ZOOM_IN_VERTICAL_
                                           << ZoomGuiCommand::ZOOM_OUT_VERTICAL_
                                           << ZoomGuiCommand::ZOOM_IN_HORIZONTAL_
                                           << ZoomGuiCommand::ZOOM_OUT_HORIZONTAL_
                                           << ZoomGuiCommand::GOTO_;


//-----------------------------------------------------------------------------
GuiActionFactoryRegistrator ZoomGuiCommand::registrator_ ("Zooming",
                                                          QSharedPointer<ZoomGuiCommand> (new ZoomGuiCommand));



//-----------------------------------------------------------------------------
ZoomGuiCommand::ZoomGuiCommand ()
    : GuiActionCommand (ACTIONS_)
{
    // nothing to do here
}

//-----------------------------------------------------------------------------
void ZoomGuiCommand::init ()
{
    getQAction (GOTO_)->setIcon (QIcon(":/images/icons/goto.png"));
    getQAction (ZOOM_IN_VERTICAL_)->setIcon (QIcon(":/images/icons/zoom_in_vertical.png"));
    getQAction (ZOOM_OUT_VERTICAL_)->setIcon (QIcon(":/images/icons/zoom_out_vertical.png"));
    getQAction (ZOOM_IN_HORIZONTAL_)->setIcon (QIcon(":/images/icons/zoom_in_horizontal.png"));
    getQAction (ZOOM_OUT_HORIZONTAL_)->setIcon (QIcon(":/images/icons/zoom_out_horizontal.png"));

    resetActionTriggerSlot (GOTO_, SLOT(goTo()));
}

//-----------------------------------------------------------------------------
void ZoomGuiCommand::trigger (QString const& action_name)
{
    QSharedPointer<MainWindowModel> main_window_model =
            ApplicationContext::getInstance()->getMainWindowModel ();
    QSharedPointer<SignalVisualisationModel> vis_model =
            main_window_model->getCurrentSignalVisualisationModel();

    unsigned shown_pos = vis_model->getShownPosition ();

    float32 pixel_per_sample = vis_model->getPixelPerSample ();
    if (action_name == ZOOM_IN_VERTICAL_)
        vis_model->zoom (ZOOM_VERTICAL, ZOOM_FACTOR_);
    else if (action_name == ZOOM_OUT_VERTICAL_)
        vis_model->zoom (ZOOM_VERTICAL, ZOOM_FACTOR_ * -1);
    else if (action_name == ZOOM_IN_HORIZONTAL_)
    {
        pixel_per_sample *= ZOOM_FACTOR_;
        if (pixel_per_sample > MAX_HORIZONTAL_ZOOM_IN_)
            pixel_per_sample = MAX_HORIZONTAL_ZOOM_IN_;
        vis_model->setPixelPerSample (pixel_per_sample);
    }
    else if (action_name == ZOOM_OUT_HORIZONTAL_)
        vis_model->setPixelPerSample (pixel_per_sample / ZOOM_FACTOR_);
    vis_model->updateLayout ();
    vis_model->goToSample (shown_pos);
    evaluateEnabledness ();
}

//-------------------------------------------------------------------------
void ZoomGuiCommand::evaluateEnabledness ()
{
    if (getApplicationState () == APP_STATE_NO_FILE_OPEN)
    {
        emit qActionEnabledChanged (false);
        return;
    }

    QSharedPointer<MainWindowModel> main_window_model =
            ApplicationContext::getInstance()->getMainWindowModel ();
    QSharedPointer<SignalVisualisationModel> vis_model =
                main_window_model->getCurrentSignalVisualisationModel();

    emit qActionEnabledChanged (true);
    if (vis_model.isNull ())
        return;

    if (vis_model->getPixelPerSample () >= MAX_HORIZONTAL_ZOOM_IN_)
        getQAction (ZOOM_IN_HORIZONTAL_)->setEnabled (false);
}


//-------------------------------------------------------------------------
void ZoomGuiCommand::goTo ()
{
    QSharedPointer<ChannelManager> channel_manager =
            ApplicationContext::getInstance()->getCurrentFileContext()->getChannelManager();
    bool ok;
    double sec = QInputDialog::getDouble (0, tr("Go to..."), tr("Second: "), 0, 0,
                             channel_manager->getDurationInSec(), 1, &ok);
    if (!ok)
        return;

    QSharedPointer<MainWindowModel> main_window_model =
            ApplicationContext::getInstance()->getMainWindowModel ();
    QSharedPointer<SignalVisualisationModel> vis_model =
                main_window_model->getCurrentSignalVisualisationModel();

    vis_model->goToSample (sec * vis_model->getSampleRate ());
}

}
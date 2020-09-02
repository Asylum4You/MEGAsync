#include "megaapi.h"
#include "OsNotifications.h"
#include "MegaApplication.h"
#include <QCoreApplication>

OsNotifications::OsNotifications(Notificator *notificator)
    :mNotificator{notificator}
{
}

QString getSharedFolderName(mega::MegaUserAlert* alert)
{
    const auto megaApi{static_cast<MegaApplication*>(qApp)->getMegaApi()};
    const auto node{megaApi->getNodeByHandle(alert->getNodeHandle())};
    auto sharedFolderName{QString::fromUtf8(node ? node->getName() : alert->getName())};
    if(!sharedFolderName.isEmpty())
    {
        return sharedFolderName;
    }
    return QCoreApplication::translate("OsNotifications", "Shared Folder Activity");
}

QString getItemsAddedText(mega::MegaUserAlert* alert)
{
    const auto updatedItems{alert->getNumber(1) + alert->getNumber(0)};
    if (updatedItems == 1)
    {
        return QCoreApplication::translate("OsNotifications", "[A] added 1 item")
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()));
    }
    else
    {
         return QCoreApplication::translate("OsNotifications", "[A] added [B] items")
                 .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()))
                 .replace(QString::fromUtf8("[B]"), QString::number(updatedItems));
    }
}

QString getItemsRemovedText(mega::MegaUserAlert* alert)
{
    const auto updatedItems{alert->getNumber(0)};
    if (updatedItems == 1)
    {
        return QCoreApplication::translate("OsNotifications", "[A] removed 1 item")
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()));
    }
    else
    {
         return QCoreApplication::translate("OsNotifications", "[A] removed [B] items")
                 .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()))
                 .replace(QString::fromUtf8("[B]"), QString::number(updatedItems));
    }
}

void OsNotifications::addUserAlertList(mega::MegaUserAlertList *alertList)
{
    for(int iAlert = 0; iAlert < alertList->size(); iAlert++)
    {
        const auto alert{alertList->get(iAlert)};
        // alerts are sent again after seen state updated, so lets only notify the unseen alerts
        if(!alert->getSeen())
        {
            auto notification{new MegaNotification()};
            switch (alert->getType())
            {
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "New Contact Request"));
                notification->setText(QCoreApplication::translate("OsNotifications","[A] sent you a contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                notification->setData(QString::fromUtf8(alert->getEmail()));
                notification->setActions(QStringList() << QCoreApplication::translate("OsNotifications", "Accept")
                                         << QCoreApplication::translate("OsNotifications", "Reject"));
                QObject::connect(notification, &MegaNotification::activated, this, &OsNotifications::incomingPendingRequest);
                break;
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "New Contact Request"));
                notification->setText(QCoreApplication::translate("OsNotifications","[A] cancelled their contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                break;
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "New Contact Request"));
                notification->setText(QCoreApplication::translate("OsNotifications", "Reminder") + QStringLiteral(": ") +
                                      QCoreApplication::translate("OsNotifications", "You have a contact request"));
                break;
            case mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "Contact Established"));
                notification->setText(QCoreApplication::translate("OsNotifications","[A] established you as a contact")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                break;
            case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "Contact Updated"));
                notification->setText(QCoreApplication::translate("OsNotifications", "You ignored a contact request"));
                break;
            case mega::MegaUserAlert::TYPE_NEWSHARE:
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(QCoreApplication::translate("OsNotifications", "New Shared folder from [X]")
                                      .replace(QString::fromUtf8("[X]"), QString::fromUtf8(alert->getEmail())));
                break;
            case mega::MegaUserAlert::TYPE_DELETEDSHARE:
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(QCoreApplication::translate("OsNotifications", "[A] has left the shared folder")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                break;
            case mega::MegaUserAlert::TYPE_NEWSHAREDNODES:
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(getItemsAddedText(alert));
                break;
            case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(getItemsRemovedText(alert));
                break;
            }
            mNotificator->notify(notification);
        }
    }
}

void OsNotifications::incomingPendingRequest(int actionId)
{
    const auto notification{static_cast<MegaNotification*>(QObject::sender())};
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    const auto requestList{megaApp->getMegaApi()->getIncomingContactRequests()};
    const auto sourceEmail{notification->getData()};
    constexpr auto acceptActionId{0};
    constexpr auto rejectActionId{1};

    for(int iRequest=0; iRequest < requestList->size(); iRequest++)
    {
        const auto request{requestList->get(iRequest)};
        if(QString::fromUtf8(request->getSourceEmail()) == sourceEmail)
        {
            if(actionId == acceptActionId)
            {
               megaApp->getMegaApi()->replyContactRequest(request, mega::MegaContactRequest::REPLY_ACTION_ACCEPT);
            }
            else if(actionId == rejectActionId)
            {
               megaApp->getMegaApi()->replyContactRequest(request, mega::MegaContactRequest::REPLY_ACTION_DENY);
            }
        }
    }
}

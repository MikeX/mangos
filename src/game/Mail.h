/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef MANGOS_MAIL_H
#define MANGOS_MAIL_H

#include "Common.h"
#include <map>

struct AuctionEntry;
class Item;
class Object;
class Player;

#define MAIL_BODY_ITEM_TEMPLATE 8383                        // - plain letter, A Dusty Unsent Letter: 889
#define MAX_MAIL_ITEMS 12

enum MailMessageType
{
    MAIL_NORMAL         = 0,
    MAIL_AUCTION        = 2,
    MAIL_CREATURE       = 3,                                // client send CMSG_CREATURE_QUERY on this mailmessagetype
    MAIL_GAMEOBJECT     = 4,                                // client send CMSG_GAMEOBJECT_QUERY on this mailmessagetype
    MAIL_ITEM           = 5,                                // client send CMSG_ITEM_QUERY on this mailmessagetype
};

enum MailCheckMask
{
    MAIL_CHECK_MASK_NONE        = 0x00,
    MAIL_CHECK_MASK_READ        = 0x01,
    MAIL_CHECK_MASK_AUCTION     = 0x04,
    MAIL_CHECK_MASK_COD_PAYMENT = 0x08,
    MAIL_CHECK_MASK_RETURNED    = 0x10
};

// gathered from Stationery.dbc
enum MailStationery
{
    MAIL_STATIONERY_UNKNOWN =  1,
    MAIL_STATIONERY_NORMAL  = 41,
    MAIL_STATIONERY_GM      = 61,
    MAIL_STATIONERY_AUCTION = 62,
    MAIL_STATIONERY_VAL     = 64,
    MAIL_STATIONERY_CHR     = 65,
    MAIL_STATIONERY_ORP     = 67,                           // new in 3.2.2
};

enum MailState
{
    MAIL_STATE_UNCHANGED = 1,
    MAIL_STATE_CHANGED   = 2,
    MAIL_STATE_DELETED   = 3
};

enum MailAuctionAnswers
{
    AUCTION_OUTBIDDED           = 0,
    AUCTION_WON                 = 1,
    AUCTION_SUCCESSFUL          = 2,
    AUCTION_EXPIRED             = 3,
    AUCTION_CANCELLED_TO_BIDDER = 4,
    AUCTION_CANCELED            = 5,
    AUCTION_SALE_PENDING        = 6
};

class MailSender
{
    public:                                                 // Constructors
        MailSender(MailMessageType messageType, uint32 sender_guidlow_or_entry, MailStationery stationery = MAIL_STATIONERY_NORMAL)
            : m_messageType(messageType), m_senderId(sender_guidlow_or_entry), m_stationery(stationery)
        {
        }
        MailSender(Object* sender, MailStationery stationery = MAIL_STATIONERY_NORMAL);
        MailSender(AuctionEntry* sender);
    public:                                                 // Accessors
        MailMessageType GetMailMessageType() const { return m_messageType; }
        uint32 GetSenderId() const { return m_senderId; }
        MailStationery GetStationery() const { return m_stationery; }
    private:
        MailMessageType m_messageType;
        uint32 m_senderId;                                  // player low guid or other object entry
        MailStationery m_stationery;
};

class MailReceiver
{
    public:                                                 // Constructors
        explicit MailReceiver(uint32 receiver_lowguid) : m_receiver(NULL), m_receiver_lowguid(receiver_lowguid) {}
        MailReceiver(Player* receiver);
        MailReceiver(Player* receiver,uint32 receiver_lowguid);
    public:                                                 // Accessors
        Player* GetPlayer() const { return m_receiver; }
        uint32  GetPlayerGUIDLow() const { return m_receiver_lowguid; }
    private:
        Player* m_receiver;
        uint32  m_receiver_lowguid;
};

class MailDraft
{
    typedef std::map<uint32, Item*> MailItemMap;

    public:                                                 // Constructors
        explicit MailDraft(uint16 mailTemplateId, bool need_items = true)
            : m_mailTemplateId(mailTemplateId), m_mailTemplateItemsNeed(need_items), m_bodyId(0), m_money(0), m_COD(0)
        {}
        MailDraft(std::string subject, uint32 itemTextId = 0)
            : m_mailTemplateId(0), m_mailTemplateItemsNeed(false), m_subject(subject), m_bodyId(itemTextId), m_money(0), m_COD(0) {}
    public:                                                 // Accessors
        uint16 GetMailTemplateId() const { return m_mailTemplateId; }
        std::string const& GetSubject() const { return m_subject; }
        uint32 GetBodyId() const { return m_bodyId; }
        uint32 GetMoney() const { return m_money; }
        uint32 GetCOD() const { return m_COD; }
    public:                                                 // modifiers
        MailDraft& AddItem(Item* item);
        MailDraft& AddMoney(uint32 money) { m_money = money; return *this; }
        MailDraft& AddCOD(uint32 COD) { m_COD = COD; return *this; }
    public:                                                 // finishers
        void SendReturnToSender(uint32 sender_acc, uint32 sender_guid, uint32 receiver_guid);
        void SendMailTo(MailReceiver const& receiver, MailSender const& sender, MailCheckMask checked = MAIL_CHECK_MASK_NONE, uint32 deliver_delay = 0);
    private:
        void deleteIncludedItems(bool inDB = false);
        void prepareItems(Player* receiver);                // called from SendMailTo for generate mailTemplateBase items

        uint16      m_mailTemplateId;
        bool        m_mailTemplateItemsNeed;
        std::string m_subject;
        uint32      m_bodyId;

        MailItemMap m_items;                                // Keep the items in a map to avoid duplicate guids (which can happen), store only low part of guid

        uint32 m_money;
        uint32 m_COD;
};

struct MailItemInfo
{
    uint32 item_guid;
    uint32 item_template;
};

struct Mail
{
    uint32 messageID;
    uint8 messageType;
    uint8 stationery;
    uint16 mailTemplateId;
    uint32 sender;
    uint32 receiver;
    std::string subject;
    uint32 itemTextId;
    std::vector<MailItemInfo> items;
    std::vector<uint32> removedItems;
    time_t expire_time;
    time_t deliver_time;
    uint32 money;
    uint32 COD;
    uint32 checked;
    MailState state;

    void AddItem(uint32 itemGuidLow, uint32 item_template)
    {
        MailItemInfo mii;
        mii.item_guid = itemGuidLow;
        mii.item_template = item_template;
        items.push_back(mii);
    }

    bool RemoveItem(uint32 item_guid)
    {
        for(std::vector<MailItemInfo>::iterator itr = items.begin(); itr != items.end(); ++itr)
        {
            if(itr->item_guid == item_guid)
            {
                items.erase(itr);
                return true;
            }
        }
        return false;
    }

    bool HasItems() const { return !items.empty(); }
};

#endif

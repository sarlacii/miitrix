#include "room.h"
#include "defines.h"
#include "request.h"

Room::Room(Matrix::RoomInfo info, std::string _roomId) {
	name = info.name;
	topic = info.topic;
	avatarUrl = info.avatarUrl;
	roomId = _roomId;
}

Room::~Room() {
	for (auto const& evt: events) {
		delete evt;
	}
}

void Room::printEvents() {
	printf_top("\x1b[2J");
	for (auto const& evt: events) {
		evt->print();
	}
}

void Room::printInfo() {
	printf_bottom("Room name: %s\n", getDisplayName().c_str());
	printf_bottom("Room topic: %s\n", topic.c_str());
}

std::string Room::getMemberDisplayName(std::string mxid) {
	std::string displayname = mxid;
	if (members.count(mxid) == 0) {
		request->getMemberInfo(mxid, roomId);
	}
	if (members[mxid].displayname != "") {
		displayname = members[mxid].displayname;
	}
	return displayname;
}

std::string Room::getDisplayName() {
	if (name != "") {
		return name;
	}
	return roomId;
}

void Room::addEvent(Event* evt) {
	// very first we claim the event as ours
	evt->setRoom(this);
	// first add the message to the internal cache
	events.push_back(evt);
	// clear unneeded stuffs
	while (events.size() > ROOM_MAX_BACKLOG) {
		delete events[0];
		events.erase(events.begin());
	}
	
	EventType type = evt->getType();
	// update the lastMsg if it is a text message
	if (type == EventType::m_room_message) {
		lastMsg = evt->getOriginServerTs();
		dirtyOrder = true;
	}

	// update room members accordingly
	if (type == EventType::m_room_member) {
		addMember(evt->getMemberMxid(), evt->getMemberInfo());
	}

	// check if we have room specific changes
	if (type == EventType::m_room_name) {
		name = evt->getRoomName();
		dirtyInfo = true;
	}
	if (type == EventType::m_room_topic) {
		topic = evt->getRoomTopic();
		dirtyInfo = true;
	}
	if (type == EventType::m_room_avatar) {
		avatarUrl = evt->getRoomAvatarUrl();
		dirtyInfo = true;
	}

	// and finally set this dirty
	dirty = true;
}

void Room::addMember(std::string mxid, Matrix::MemberInfo m) {
	members[mxid] = m;
	dirty = true;
}

u32 Room::getLastMsg() {
	return lastMsg;
}

bool Room::haveDirty() {
	return dirty;
}

bool Room::haveDirtyInfo() {
	return dirtyInfo;
}

bool Room::haveDirtyOrder() {
	return dirtyOrder;
}

void Room::resetDirty() {
	dirty = false;
}

void Room::resetDirtyInfo() {
	dirtyInfo = false;
}

void Room::resetDirtyOrder() {
	dirtyOrder = false;
}

std::string Room::getId() {
	return roomId;
}

void Room::updateInfo(Matrix::RoomInfo info) {
	name = info.name;
	topic = info.topic;
	avatarUrl = info.avatarUrl;
	dirty = true;
	dirtyInfo = true;
}
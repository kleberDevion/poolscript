package com.intellij.notification;

public interface NotificationGroupManager {
  static NotificationGroupManager getInstance() { return null; }
  NotificationGroup getNotificationGroup(String groupId);
}

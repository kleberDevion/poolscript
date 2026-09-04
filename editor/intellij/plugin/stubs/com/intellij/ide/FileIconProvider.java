package com.intellij.ide;
public interface FileIconProvider {
  javax.swing.Icon getIcon(com.intellij.openapi.vfs.VirtualFile file, int flags,
                           com.intellij.openapi.project.Project project);
}

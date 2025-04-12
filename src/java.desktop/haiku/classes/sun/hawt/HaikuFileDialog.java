/*
 * Copyright (c) 2012, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

package sun.hawt;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import java.awt.image.*;
import java.util.List;
import java.io.*;

import sun.awt.*;
import sun.java2d.pipe.Region;

class HaikuFileDialog implements FileDialogPeer {

    static {
        initIDs();
    }

    private final FileDialog target;
    private boolean done;

    private native static void initIDs();
    private native void nativeShowDialog(String title, boolean saveMode,
        boolean multipleMode, boolean filterFilenames, String directory);

    HaikuFileDialog(FileDialog target) {
        this.target = target;
        this.done = false;
    }

    @Override
    public void dispose() {
        HaikuToolkit.targetDisposedPeer(target, this);
    }

    private void run() {
        if (done) {
            throw new IllegalStateException("FileDialog can only run once");
        }

        boolean saveMode = target.getMode() == FileDialog.SAVE;

        String title = target.getTitle();
        if (title == null) {
            if (saveMode) {
                title = "Save";
            } else {
                title = "Open";
            }
        }

        nativeShowDialog(title, saveMode, target.isMultipleMode(),
            target.getFilenameFilter() != null, target.getDirectory());
    }

    @Override
    public void setVisible(boolean visible) {
        if (visible) {
            run();
        }
        // The Mac version of this doesn't bother with setVisible(false),
        // probably because setVisible(true) blocks. Following suit for now.
    }

    /*
     * Upcall from native code to determine whether the given filename
     * should be accepted or not. Returns true if acceptable, and false
     * if the file should be filtered out.
     */
    private boolean acceptFile(String filename) {
        FilenameFilter filter = target.getFilenameFilter();
        if (filter == null)
            return true;

        File file = new File(filename);
        if (file.isDirectory())
            return true;

        File directory = new File(file.getParent());
        String name = file.getName();
        return filter.accept(directory, name);
    }

    /*
     * Upcall from native code indicating that the dialog has been
     * closed.
     */
    private void done(String[] filenames) {
        // We get another B_CANCEL message when the dialog closes, even after
        // a save or open, so we ignore it.
        if (done) {
            return;
        }

        done = true;

        String directory = null;
        String file = null;
        File[] files = null;

        try {
            if (filenames != null) {
                // the dialog wasn't cancelled
                int count = filenames.length;
                files = new File[count];
                for (int i = 0; i < count; i++) {
                    files[i] = new File(filenames[i]);
                }
            
                directory = files[0].getParent();
                // make sure directory always ends in '/'
                if (!directory.endsWith(File.separator)) {
                    directory = directory + File.separator;
                }
            
                file = files[0].getName(); // pick any file
            }
            
            // store results back in component
            AWTAccessor.FileDialogAccessor accessor = AWTAccessor.getFileDialogAccessor();
            accessor.setDirectory(target, directory);
            accessor.setFile(target, file);
            accessor.setFiles(target, files);
        } finally {
            target.dispose();
        }

        // The native resources are already disposed
    }

    /*
     * These methods are unimplemented because we get all this stuff from
     * the target when we are about to be shown.
     */

    @Override
    public void setDirectory(String directory) {
    }

    @Override
    public void setFile(String filename) {
    }

    @Override
    public void setFilenameFilter(FilenameFilter filter) {
    }

    /*
     * The Windows and Mac toolkits don't implement any of the below
     * functionality, so it's unlikely any apps rely on it.
     */

    @Override
    public void blockWindows(List<Window> windows) {
    }

    @Override
    public void setResizable(boolean resizeable) {
    }

    @Override
    public void setTitle(String title) {
    }

    @Override
    public void repositionSecurityWarning() {
    }

    @Override
    public void updateAlwaysOnTopState() {
    }

    @Override
    public void setModalBlocked(Dialog blocker, boolean blocked) {
    }

    @Override
    public void setOpacity(float opacity) {
    }

    @Override
    public void setOpaque(boolean isOpaque) {
    }

    @Override
    public void toBack() {
    }

    @Override
    public void toFront() {
    }

    @Override
    public void updateFocusableWindowState() {
    }

    @Override
    public void updateIconImages() {
    }

    @Override
    public void updateMinimumSize() {
    }

    @Override
    public void updateWindow() {
    }

    @Override
    public void beginLayout() {
    }

    @Override
    public void beginValidate() {
    }

    @Override
    public void endLayout() {
    }

    @Override
    public void endValidate() {
    }

    @Override
    public Insets getInsets() {
        return new Insets(0, 0, 0, 0);
    }

    @Override
    public void applyShape(Region shape) {
    }

    @Override
    public boolean canDetermineObscurity() {
        return false;
    }

    @Override
    public void coalescePaintEvent(PaintEvent e) {
    }

    @Override
    public void createBuffers(int numBuffers, BufferCapabilities caps)
            throws AWTException {
    }

    @Override
    public Image createImage(int width, int height) {
        return null;
    }

    @Override
    public VolatileImage createVolatileImage(int width, int height) {
        return null;
    }

    @Override
    public void destroyBuffers() {
    }

    @Override
    public void flip(int x1, int y1, int x2, int y2,
            BufferCapabilities.FlipContents flipAction) {
    }

    @Override
    public Image getBackBuffer() {
        return null;
    }

    @Override
    public ColorModel getColorModel() {
        return null;
    }

    @Override
    public FontMetrics getFontMetrics(Font font) {
        return null;
    }

    @Override
    public Graphics getGraphics() {
        return null;
    }

    @Override
    public GraphicsConfiguration getGraphicsConfiguration() {
        return null;
    }

    @Override
    public Point getLocationOnScreen() {
        return null;
    }

    @Override
    public Dimension getMinimumSize() {
        return target.getSize();
    }

    @Override
    public Dimension getPreferredSize() {
        return getMinimumSize();
    }

    @Override
    public void handleEvent(AWTEvent e) {
    }

    @Override
    public boolean handlesWheelScrolling() {
        return false;
    }

    @Override
    public boolean isFocusable() {
        return false;
    }

    @Override
    public boolean isObscured() {
        return false;
    }

    @Override
    public boolean isReparentSupported() {
        return false;
    }

    @Override
    public void layout() {
    }

    @Override
    public void paint(Graphics g) {
    }

    @Override
    public void print(Graphics g) {
    }

    @Override
    public void reparent(ContainerPeer newContainer) {
    }

    @Override
    public boolean requestFocus(Component lightweightChild, boolean temporary,
            boolean focusedWindowChangeAllowed, long time,
            FocusEvent.Cause cause) {
        return false;
    }

    @Override
    public void setBackground(Color c) {
    }

    @Override
    public void setForeground(Color c) {
    }

    @Override
    public void setBounds(int x, int y, int width, int height, int op) {
    }

    @Override
    public void setEnabled(boolean e) {
    }

    @Override
    public void setFont(Font f) {
    }

    @Override
    public void setZOrder(ComponentPeer above) {
    }

    @Override
    public void updateCursorImmediately() {
    }

    @Override
    public boolean updateGraphicsData(GraphicsConfiguration gc) {
        return false;
    }
}

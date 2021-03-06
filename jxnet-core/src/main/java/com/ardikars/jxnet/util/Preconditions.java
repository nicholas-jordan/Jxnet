/**
 * Copyright (C) 2017  Ardika Rommy Sanjaya
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.ardikars.jxnet.util;

/**
 * @author Ardika Rommy Sanjaya
 * @since 1.1.0
 */
public final class Preconditions {
	
	/**
     * Check argument.
     * @param expression false will show error message.
     */
	public static native void CheckArgument(boolean expression);
	
    /**
     * Check argument.
     * @param expression false will show error message.
     * @param errorMessage error message.
     */
    public static native void CheckArgument(boolean expression, String errorMessage);

    /**
     * Check state
     * @param expression false will show error message.
     */
    public static native void CheckState(boolean expression);
    
    /**
     * Check state
     * @param expression false will show error message.
     * @param errorMessage error message.
     */
    public static native void CheckState(boolean expression, String errorMessage);

	/**
     * Check not null.
     * @param reference false will show error message.
     * @param <T> object type.
     * @return same object.
     */
    public static native <T> T CheckNotNull(T reference);

    /**
     * Check not null.
     * @param reference false will show error message.
     * @param errorMessage error message.
     * @param <T> object type.
     * @return same object.
     */
    public static native <T> T CheckNotNull(T reference, String errorMessage);

    static {
        try {
            Class.forName("com.ardikars.jxnet.Jxnet");
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

}

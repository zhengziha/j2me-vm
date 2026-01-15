#!/bin/bash
javac -Xlint:-options -source 1.4 -target 1.4 -d . java/io/PrintStream.java
javac -Xlint:-options -source 1.4 -target 1.4 -d . java/lang/System.java
javac -Xlint:-options -source 1.4 -target 1.4 -d . java/lang/AbstractStringBuilder.java
javac -Xlint:-options -source 1.4 -target 1.4 -d . java/lang/StringBuilder.java

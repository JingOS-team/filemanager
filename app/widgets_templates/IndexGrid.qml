import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import org.kde.maui 1.0 as Maui

GridView
{
    id: folderGridRoot

    property int itemSize : iconSizes.large
    property int itemSpacing: itemSize * 0.5 + (isMobile ? space.big :
                                                           space.huge)

    signal itemClicked(int index)
    signal itemDoubleClicked(int index)

    flow: GridView.FlowLeftToRight
    clip: true
    width: parent.width
    height: parent.height

    //    cacheBuffer: cellHeight * 2

    cellWidth: itemSize + itemSpacing
    cellHeight: itemSize + itemSpacing

    focus: true

    boundsBehavior: Flickable.StopAtBounds
    flickableDirection: Flickable.AutoFlickDirection
    snapMode: GridView.SnapToRow

    model: ListModel { id: gridModel  }

    delegate: Maui.IconDelegate
    {
        id: delegate

        isDetails: false
        width: cellWidth * 0.9
        height: cellHeight * 0.9
        folderSize : itemSize
        showTooltip: true
        showThumbnails: browser.previews

        Connections
        {
            target: delegate
            onClicked:
            {
                folderGridRoot.currentIndex = index
                folderGridRoot.itemClicked(index)
            }

            onDoubleClicked:
            {
                folderGridRoot.currentIndex = index
                itemDoubleClicked(index)
            }

            onPressAndHold:
            {
                folderGridRoot.currentIndex = index
                itemMenu.show(folderGridRoot.model.get(index).path)
            }

            onRightClicked:
            {
                folderGridRoot.currentIndex = index
                itemMenu.show(folderGridRoot.model.get(index).path)
            }

            onEmblemClicked:
            {
                var item = folderGridRoot.model.get(index)
                browser.addToSelection(item, true)
            }
        }
    }

    ScrollBar.vertical: ScrollBar{ visible: true}

    onWidthChanged: adaptGrid()

    MouseArea
    {
        anchors.fill: parent
        z: -1
        acceptedButtons:  Qt.RightButton | Qt.LeftButton
        onClicked:
        {
            if(!isMobile && mouse.button === Qt.RightButton)
                browserMenu.show()

        }

        onPressAndHold: browserMenu.show()
    }

    function adaptGrid()
    {
        var amount = parseInt(width/(itemSize + itemSpacing),10)
        var leftSpace = parseInt(width-(amount*(itemSize + itemSpacing)), 10)
        var size = parseInt((itemSize + itemSpacing)+(parseInt(leftSpace/amount, 10)), 10)

        size = size > itemSize + itemSpacing ? size : itemSize + itemSpacing

        cellWidth = size
    }
}
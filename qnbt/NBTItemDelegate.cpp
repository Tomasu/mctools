#include <QSpinBox>
#include <QLineEdit>
#include "NBTItemDelegate.h"

#include "NBT_Tag.h"
#include "NBT_Tag_Byte.h"
#include "NBT_Tag_Short.h"
#include "NBT_Tag_Int.h"
#include "NBT_Tag_Long.h"
#include "NBT_Tag_Float.h"
#include "NBT_Tag_Double.h"
#include "NBT_Tag_String.h"

NBTItemDelegate::NBTItemDelegate(QObject *parent) :
	QItemDelegate(parent)
{
}

QWidget *NBTItemDelegate::createNumberEditor(uint32_t bits, QWidget *parent) const
{
	QSpinBox *editor = new QSpinBox(parent);
	return editor;
}

void NBTItemDelegate::updateNumberEditor(uint32_t bits, QWidget *widget, uint64_t value) const
{
	QSpinBox *editor = static_cast<QSpinBox *>(widget);

	editor->setMaximum((1 << (bits-1))-1);
	editor->setMinimum(-(1 << (bits-1)));
	editor->setValue(value);
}

QWidget *NBTItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	NBT_Tag *tag = static_cast<NBT_Tag *>(index.internalPointer());
	QWidget *editor = 0;

	//if(index.column() != 2)
	//	return 0;

	switch(tag->type())
	{
		case TAG_Byte:
			editor = createNumberEditor(8, parent);
			break;

		case TAG_Short:
			editor = createNumberEditor(16, parent);
			break;

		case TAG_Int:
			editor = createNumberEditor(32, parent);
			break;

		case TAG_Long:
			editor = createNumberEditor(64, parent);
			break;

		case TAG_Float:
		case TAG_Double:
		case TAG_String:
		{
			//NBT_Tag_String *string = static_cast<NBT_Tag_String *>(tag);
			editor = new QLineEdit(/*string->value().c_str(),*/ parent);
		} break;
	}

	return editor;
}

void NBTItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	NBT_Tag *tag = static_cast<NBT_Tag *>(index.internalPointer());

	if(index.column() != 2)
		return;

	switch(tag->type())
	{
		case TAG_Byte:
			updateNumberEditor(8, editor, static_cast<NBT_Tag_Byte *>(tag)->value());
			break;

		case TAG_Short:
			updateNumberEditor(16, editor, static_cast<NBT_Tag_Short *>(tag)->value());
			break;

		case TAG_Int:
			updateNumberEditor(32, editor, static_cast<NBT_Tag_Int *>(tag)->value());
			break;

		case TAG_Long:
			updateNumberEditor(64, editor, static_cast<NBT_Tag_Long *>(tag)->value());
			break;

		case TAG_Float:
		{
			NBT_Tag_Float *ftag = static_cast<NBT_Tag_Float *>(tag);
			static_cast<QLineEdit *>(editor)->setText(QString::number(ftag->value()));
		} break;

		case TAG_Double:
		{
			NBT_Tag_Double *dtag = static_cast<NBT_Tag_Double *>(tag);
			static_cast<QLineEdit *>(editor)->setText(QString::number(dtag->value()));
		} break;

		case TAG_String:
		{
			NBT_Tag_String *string = static_cast<NBT_Tag_String *>(tag);
			static_cast<QLineEdit *>(editor)->setText(string->value().c_str());
		} break;
	}

}

void NBTItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	NBT_Tag *tag = static_cast<NBT_Tag *>(index.internalPointer());

	if(index.column() != 2)
		return;

	switch(tag->type())
	{
		case TAG_Byte:
		case TAG_Short:
		case TAG_Int:
		case TAG_Long:
		{
			QSpinBox *box = static_cast<QSpinBox *>(editor);
			box->interpretText();
			qulonglong value = box->value();
			model->setData(index, QVariant(value), Qt::EditRole);
		} break;

		case TAG_Float:
		case TAG_Double:
		case TAG_String:
		{
			QLineEdit *edit = static_cast<QLineEdit *>(editor);
			model->setData(index, edit->text(), Qt::EditRole);
		} break;
	}
}

void NBTItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QRect adjust = option.rect;
	adjust.setHeight(adjust.height()+14);
	adjust.moveTop(adjust.top()-4);
	editor->setGeometry(adjust);
}

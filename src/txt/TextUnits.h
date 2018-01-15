#pragma once

#include "cinder/app/App.h"

namespace txt
{
	enum UnitType {
		PX,
		PT,
		EM,
		PCT
	};

	class Unit
	{
		public:
			Unit() :
				mType( UnitType::PX ),
				mIsDefault( true ) {}

			Unit( float value, UnitType type = UnitType::PX )
			{
				Unit::Unit();
				setValue( value, type );
			}

			void setValue( float value, UnitType type = UnitType::PX )
			{
				mValue = value;
				mType = type;
				mIsDefault = false;
			}

			float getValue( float baseValue ) const
			{
				switch( mType ) {
					case PX:
						return mValue;

					case PT:
						return mValue / ci::app::getWindowContentScale(); // Get Cinder's window scale and apply here

					case EM:
					case PCT:
						return mValue * baseValue;
				}
			}

			UnitType getValueType() const { return mType; }

			bool isDefault() const { return mIsDefault; }

		private:
			float mValue;
			UnitType mType;
			bool mIsDefault;
	};
}
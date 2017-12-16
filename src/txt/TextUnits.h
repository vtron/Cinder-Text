#pragma once


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

			float getValue( float baseValue )
			{
				switch( mType ) {
					case PX:
						return mValue;

					case PT:
						return mValue; // Get Cinder's window scale and apply here

					case EM:
					case PCT:
						return mValue * baseValue;
				}

			}
			UnitType getValueType() { return mType; }

			bool isDefault() { return mIsDefault; }

		private:
			float mValue;
			UnitType mType;
			bool mIsDefault;
	};
}